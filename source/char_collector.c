#include "../include/text_builder.h"
#include <math.h>
#include <mimalloc.h>
#include <string.h>
#include <inttypes.h> // for PRIdMAX
#include <time.h>

#define U_USING_ICU_NAMESPACE  0
#include <unicode/utypes.h>
#include <unicode/ubrk.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
#include <unicode/utf8.h>
#include <unicode/uclean.h>

#include "string_hash_table.h"
struct _QOR_GraphemeCollector
{
    _FunnelHashTable  ht;
    _OrderList        order_list;
    UErrorCode        last_error;
    qo_ref_count_t    ref_count;
};

qo_stat_t
grapheme_collector_new(
    QOR_GraphemeCollector ** p_self ,
    qo_size_t                capacity ,
    qo_fp32_t                funnel_delta
) {
    QOR_GraphemeCollector * self =
        mi_malloc(sizeof(QOR_GraphemeCollector));
    if (!self)
    {
        return NULL;
    }

    qo_stat_t  ret;
    ret = funnel_hash_table_init(&self->ht , capacity , funnel_delta);
    if (ret != QO_OK)
    {
        mi_free(self);
        return NULL;
    }

    qo_size_t  list_capacity = (qo_size_t) fmax(32.0f ,
        (qo_fp32_t) self->ht.max_inserts);
    ret = order_list_init(&self->order_list , list_capacity);
    if (ret != QO_OK)
    {
        funnel_hash_table_destroy(&self->ht);
        mi_free(self);
        return NULL;
    }

    self->last_error = U_ZERO_ERROR;
    *p_self = self;
    return QO_OK;
}

void
qor_grapheme_collector_delete(
    QOR_GraphemeCollector * self
) {
    if (self)
    {
        order_list_destroy(&self->order_list);
        funnel_hash_table_destroy(&self->ht);
        mi_free(self);
    }
}

qo_stat_t
qor_sample_utf16_grapheme(
    QOR_GraphemeCollector * self ,
    UChar const *           text ,
    qo_size_t               text_len
) {
    if (!self || !text || !text_len)
    {
        return QO_INVALID_ARG;
    }

    qo_bool_t  overall_success = true;

    UErrorCode  status = U_ZERO_ERROR;
    UNormalizer2 const * nfc_normalizer = unorm2_getNFCInstance(&status);
    if (U_FAILURE(status))
    {
        self->last_error = status;
        return QO_UNKNOWN_ERROR;
    }

    status = U_ZERO_ERROR;
    qo_int32_t  normalized_utf16_len =
        unorm2_normalize(nfc_normalizer , text , text_len , NULL , 0 , &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR)
    {
        self->last_error = status;
        return QO_UNKNOWN_ERROR;
    }

    // Normalized string is empty
    if (normalized_utf16_len <= 0)
    {
        return QO_INVALID_ARG;
    }

    UChar * normalized_utf16_buffer = (UChar *) mi_mallocn_tp(UChar ,
        normalized_utf16_len);
    if (!normalized_utf16_buffer)
    {
        self->last_error = U_MEMORY_ALLOCATION_ERROR;
        return QO_OUT_OF_MEMORY;
    }

    status = U_ZERO_ERROR;
    unorm2_normalize(nfc_normalizer , text , text_len ,
        normalized_utf16_buffer , normalized_utf16_len , &status);
    if (U_FAILURE(status))
    {
        self->last_error = status;
        mi_free(normalized_utf16_buffer);
        return QO_UNKNOWN_ERROR;
    }

    status = U_ZERO_ERROR;
    UBreakIterator * break_iterator = ubrk_open(UBRK_CHARACTER , "" ,
        normalized_utf16_buffer , normalized_utf16_len , &status);
    if (U_FAILURE(status) || !break_iterator)
    {
        self->last_error = status;
        mi_free(normalized_utf16_buffer);
        return QO_UNKNOWN_ERROR;
    }

    qo_int32_t  start_boundary = ubrk_first(break_iterator);
    qo_int32_t  end_boundary = 0;
    qo_cstring_t  current_grapheme_utf8 = NULL;

    while ((end_boundary = ubrk_next(break_iterator)) != UBRK_DONE)
    {
        qo_int32_t  grapheme_utf16_len = end_boundary - start_boundary;
        if (grapheme_utf16_len > 0)
        {
            qo_int32_t  grapheme_utf8_len = 0;
            current_grapheme_utf8 = NULL;

            // Calculate the buffer size needed for conversion to UTF-8
            status = U_ZERO_ERROR;
            grapheme_utf8_len = u_strToUTF8(NULL , 0 , &grapheme_utf8_len ,
                normalized_utf16_buffer + start_boundary , grapheme_utf16_len ,
                &status);
            if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status))
            {
                self->last_error = status;
                ubrk_close(break_iterator);
                mi_free(normalized_utf16_buffer);
                return QO_UNKNOWN_ERROR;
            }

            // 分配UTF-8缓冲区
            current_grapheme_utf8 = (qo_cstring_t) mi_malloc(
                grapheme_utf8_len + 1
            );
            if (!current_grapheme_utf8)
            {
                self->last_error = U_MEMORY_ALLOCATION_ERROR;
                ubrk_close(break_iterator);
                mi_free(normalized_utf16_buffer);
                return QO_OUT_OF_MEMORY;
            }

            // 执行UTF-16到UTF-8的转换
            status = U_ZERO_ERROR;
            u_strToUTF8(current_grapheme_utf8 , grapheme_utf8_len + 1 , NULL ,
                normalized_utf16_buffer + start_boundary , grapheme_utf16_len ,
                &status);
            if (U_FAILURE(status))
            {
                self->last_error = status;
                mi_free(current_grapheme_utf8);
                ubrk_close(break_iterator);
                mi_free(normalized_utf16_buffer);
                return QO_UNKNOWN_ERROR;
            }

            // I tested on another individual case and it seems unnecessary
            // current_grapheme_utf8[grapheme_utf8_len] = '\0';

            qo_cstring_t  grapheme_copy_for_table =
                mi_strdup(current_grapheme_utf8);
            if (!grapheme_copy_for_table)
            {
                self->last_error = U_MEMORY_ALLOCATION_ERROR;
                mi_free(current_grapheme_utf8);
                current_grapheme_utf8 = NULL;
                break;
            }

            if (funnel_hash_table_insert(&self->ht , grapheme_copy_for_table ,
                ENTRY_VALUE_DUMMY))
            {
                qo_cstring_t  grapheme_copy_for_list =
                    mi_strdup(current_grapheme_utf8);
                if (!grapheme_copy_for_list)
                {
                    overall_success = false;
                    self->last_error = U_MEMORY_ALLOCATION_ERROR;
                    mi_free(current_grapheme_utf8);
                    current_grapheme_utf8 = NULL;
                    break;
                }

                if (!order_list_add(&self->order_list ,
                    grapheme_copy_for_list))
                {
                    overall_success = false;
                    self->last_error = U_MEMORY_ALLOCATION_ERROR;
                    mi_free(current_grapheme_utf8);
                    mi_free(grapheme_copy_for_list);
                    current_grapheme_utf8 = NULL;
                    break;
                }
            }
            else  // Insertion failed
            {
                free(grapheme_copy_for_table);
                grapheme_copy_for_table = NULL;
            }

            mi_free(current_grapheme_utf8); // mi_free can safely handle nullptr
            current_grapheme_utf8 = NULL;
        }

        start_boundary = end_boundary;
    } // End of while loop

    // They are all safe to pass nullptr
    ubrk_close(break_iterator);
    mi_free(normalized_utf16_buffer);
    mi_free(current_grapheme_utf8);
    return overall_success ? QO_OK : QO_UNKNOWN_ERROR;
}

// ... existing code ...
qo_stat_t
qor_sample_utf8_grapheme(
    QOR_GraphemeCollector * self ,
    qo_ccstring_t           utf8_text ,
    qo_size_t               utf8_len
) {
    if (!self || !utf8_text || !utf8_len)
    {
        return QO_INVALID_ARG;
    }
    self->last_error = U_ZERO_ERROR;

    UErrorCode  status = U_ZERO_ERROR;
    UChar * utf16_buffer  = NULL;
    qo_int32_t  utf16_len = 0;

    // Get the buffer size needed for conversion to UTF-16
    u_strFromUTF8(NULL , 0 , &utf16_len , utf8_text , utf8_len , &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status))
    {
        self->last_error = status;
        return QO_UNKNOWN_ERROR;
    }
    // We receive an empty string
    if (utf16_len <= 0)
    {
        return QO_INVALID_ARG;
    }

    utf16_buffer = (UChar *) mi_mallocn_tp(UChar , utf16_len + 1);
    if (!utf16_buffer)
    {
        self->last_error = U_MEMORY_ALLOCATION_ERROR;
        return QO_OUT_OF_MEMORY;
    }

    // Perform true conversion
    status = U_ZERO_ERROR;
    u_strFromUTF8(utf16_buffer , utf16_len + 1 , NULL , utf8_text , utf8_len ,
        &status);
    if (U_FAILURE(status))
    {
        self->last_error = status;
        mi_free(utf16_buffer);
        return QO_UNKNOWN_ERROR;
    }

    // Notice: the converted string is NOT normalized
    // So we let qor_sample_utf16_grapheme do it
    qo_stat_t  ret = qor_sample_utf16_grapheme(self , utf16_buffer ,
        utf16_len);
    mi_free(utf16_buffer);
    return ret;
}

qo_stat_t
qor_grapheme_collector_view(
    QOR_GraphemeCollector *     self ,
    QOR_GraphemeCollectorView * view
) {
    view->graphemes = (qo_ccstring_t *) self->order_list.items;
    view->count = self->order_list.size;
    return QO_OK;
}
