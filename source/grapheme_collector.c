#include "../include/text_builder.h"
#include <cstddef>
#include <math.h>
#include <mimalloc.h>
#include <string.h>
#include <inttypes.h> // for PRIdMAX
#include <time.h>

#define U_USING_ICU_NAMESPACE  0
#define U_DISABLE_RENAMING     1
#include <unicode/utypes.h>
#include <unicode/ubrk.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
#include <unicode/utf8.h>
#include <unicode/uclean.h>

#include "grapheme_collect.h"
#include "funnel_hash_table.h"
#include "ordered_list.h"
#include "D:\cpp\QuickOK-Zero\include\stringzilla.h" // TODO: Move to dep

qo_uint64_t
hash_u16string(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    UChar const * ustr = (const UChar*)key;
    if (!ustr) return (uint64_t)salt;

    // FNV-1a hash implementation for UChar*
    qo_uint64_t hash = 14695981039346656037ULL;
    while (*ustr) {
        hash ^= (uint64_t)(*ustr++);
        hash *= 1099511628211ULL;
    }
    return hash ^ (uint64_t)salt;
}

qo_bool_t
u16string_equals(
    fht_key_t  key1 ,
    fht_key_t  key2
) {
    return !u_strcmp((UChar const *)key1 , (UChar const *)key2);
}

void
destroy_string_key(
    fht_key_t  key
) {
    mi_free((qo_cstring_t) key);
}

#define ZERO_STATUS(s)   (s) = U_ZERO_ERROR
#define ERROR_RETURN(s)  self->internal_error = (s); return qo_false

qo_bool_t
collect_urf16_graphemes(
    _GraphemeCollector * self ,
    UChar const *        str ,
    qo_size_t            len
) {
    UErrorCode  status = U_ZERO_ERROR;
    UNormalizer2 const * nfc_normalizer = unorm2_getNFCInstance(&status);
    if (U_FAILURE(status))
    {
        ERROR_RETURN(status);
    }

    ZERO_STATUS(status);
    qo_int32_t normalized_len = unorm2_normalize(nfc_normalizer , str , len ,
        NULL , 0 , &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR)
    {
        ERROR_RETURN(status);
    }
    if (normalized_len <= 0) // Nothing to do
    {
        return qo_true;
    }

    UChar * normalized_buffer = mi_mallocn_tp(UChar , normalized_len);
    if (!normalized_buffer)
    {
        ERROR_RETURN(U_MEMORY_ALLOCATION_ERROR);
    }

    ZERO_STATUS(status);
    unorm2_normalize(nfc_normalizer , str , len , normalized_buffer , normalized_len , &status);
    if (U_FAILURE(status))
    {
        mi_free(normalized_buffer);
        ERROR_RETURN(status);
    }

    ZERO_STATUS(status);
    UBreakIterator * break_iterator = ubrk_open(UBRK_CHARACTER , "" , normalized_buffer , normalized_len , &status); 
    if (U_FAILURE(status) || !break_iterator)
    {
        mi_free(normalized_buffer);
        ERROR_RETURN(status);
    }

    qo_bool_t overall_success = qo_true;
    qo_int32_t start_boundary = ubrk_first(break_iterator);
    qo_int32_t end_boundary;
    while ((end_boundary = ubrk_next(break_iterator)) != UBRK_DONE)
    {
        qo_int32_t grapheme_len = end_boundary - start_boundary;
        if (grapheme_len > 0)
        {
            UChar const * current_grapheme = normalized_buffer + start_boundary;
            UChar original_char = normalized_buffer[end_boundary];
            normalized_buffer[end_boundary] = 0;

            // When it not exists
            if(!fht_search(&self->hash_table , current_grapheme , NULL))
            {
                UChar * grapheme_copy = mi_mallocn_tp(UChar , grapheme_len + 1);
                if (!grapheme_copy)
                {
                    overall_success = qo_false;
                    self->internal_error = U_MEMORY_ALLOCATION_ERROR;
                    normalized_buffer[end_boundary] = original_char; // Restore it
                    break;
                }
                u_strncpy(grapheme_copy , current_grapheme , grapheme_len);
                grapheme_copy[grapheme_len] = 0;

                if (fht_insert(&self->hash_table , grapheme_copy , 1))
                {
                    if (!ordered_list_add(&self->ordered_list, grapheme_copy))
                    {
                        overall_success = qo_false;
                        self->internal_error = U_MEMORY_ALLOCATION_ERROR;
                        normalized_buffer[end_boundary] = original_char; // Restore it
                    }
                }
                else {
                    mi_free(grapheme_copy);
                    overall_success = qo_false;
                    self->internal_error = U_MEMORY_ALLOCATION_ERROR;
                    normalized_buffer[end_boundary] = original_char;
                    break;
                }
                normalized_buffer[end_boundary] = original_char;
            }

            start_boundary = end_boundary;
        }
    }

    ubrk_close(break_iterator);
    mi_free(normalized_buffer);
    return overall_success;
}

qo_bool_t
collect_utf8_graphemes(
    _GraphemeCollector *    self,
    qo_ccstring_t           str ,
    qo_size_t               len //< TODO: Use it
) {
    ZERO_STATUS(self->internal_error);
    UErrorCode status = U_ZERO_ERROR;
    qo_int32_t  utf16_len;

    u_strFromUTF8(NULL, 0, &utf16_len, str, -1, &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR)
    {
        ERROR_RETURN(status);
    }
    if (utf16_len <= 0) // Nothing to do
    {
        return qo_true;
    }

    UChar * utf16_buffer = mi_mallocn_tp(UChar , utf16_len + 1);
    if (!utf16_buffer)
    {
        ERROR_RETURN(U_MEMORY_ALLOCATION_ERROR);
    }

    ZERO_STATUS(status);
    u_strFromUTF8(utf16_buffer, utf16_len + 1, &utf16_len, str, -1, &status);
    if (U_FAILURE(status))
    {
        mi_free(utf16_buffer);
        ERROR_RETURN(status);
    }

    qo_bool_t success = collect_urf16_graphemes(self, utf16_buffer, utf16_len);
    mi_free(utf16_buffer);
    return success;
}


_GraphemeCollectorIterator
grapheme_collector_iterate(
    _GraphemeCollector const *    self
) {
    _GraphemeCollectorIterator iter = {
        .list = &self->ordered_list,
        .current_index = 0
    };
    return iter;
}

qo_bool_t
grapheme_collector_iterator_next(
    _GraphemeCollectorIterator *    self ,
    UChar **                        p_grapheme ,
    qo_int32_t *                    p_len
) {
    if (self->current_index >= ordered_list_get_count(self->list))
    {
        return qo_false;
    }

    UChar const * grapheme = ordered_list_get(self->list , self->current_index);
    if (p_grapheme)
    {
        *p_grapheme = (UChar *)grapheme;
    }
    if (p_len)
    {
        *p_len = u_strlen(grapheme);
    }
    self->current_index++;
    return qo_true;
}