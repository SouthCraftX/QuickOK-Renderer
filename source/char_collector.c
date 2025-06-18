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

#include "../../QuickOK-Zero/include/stringzilla.h"
#include "string_hash_table.h"
struct _QOR_GraphemeCollector
{
    _FunnelHashTable  ht;
    _OrderList        order_list;
    UErrorCode        last_error;
    qo_ref_count_t    ref_count;
};

qo_stat_t
// returns true if failed
order_list_init(
    _OrderList * list ,
    qo_size_t    initial_capacity
) {
    initial_capacity = initial_capacity ? initial_capacity : 8;
    list->items = (qo_cstring_t *) mi_malloc(
        initial_capacity * sizeof(qo_cstring_t)
    );

    if (list->items)
    {
        list->size = 0;
        list->capacity = initial_capacity;
        return QO_OK;
    }
    return QO_OUT_OF_MEMORY;
}

void
order_list_destroy(
    _OrderList * list
) {
    for (qo_size_t i = 0; i < list->size; i++)
    {
        mi_free(list->items[i]);
    }
    mi_free(list->items);
}

qo_bool_t
// returns true if failed
order_list_add(
    _OrderList *  list ,
    qo_cstring_t  item
) {
    if (list->size == list->capacity)
    {
        qo_size_t  new_capacity = list->capacity ? list->capacity * 2 : 8;
        qo_cstring_t * new_items = mi_reallocn_tp(list->items, qo_cstring_t, new_capacity);

        if (!new_items)
        {
            return qo_true;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->size++] = item; // take ownership of item
    return qo_false;
}

qo_uint64_t
funnel_sz_hash_initial(
    qo_ccstring_t  str ,
    qo_uint32_t    salt
) {
    if (!str)
    {
        return (qo_uint64_t) salt;      // Handle NULL string input
    }
    qo_size_t  len = strlen(str);
    sz_u64_t   h64 = sz_hash(str , len);
    return h64 ^ (qo_uint64_t) salt;
}

qo_uint64_t
funnel_sz_hash_step(
    qo_ccstring_t  str
) {
    if (!str)
    {
        return 1;
    }
    qo_size_t  len = strlen(str);
    sz_u64_t   h64 = sz_hash(str , len);
    const qo_uint64_t  prime_multiplier = 0xc6a4a7935bd1e995ULL;
    qo_uint64_t  step64 = (h64 * prime_multiplier) ^ (h64 >> 31);
    return step64 | 1;
}

qo_size_t
calculate_probe_limit(
    qo_size_t  capacity
) {
    if (capacity < 2)
    {
        return 1;
    }
    return (qo_size_t) ceilf(logf((qo_fp32_t) capacity) * 3.0f);
}

// Cleanup function adapted for strings
static void
funnel_levels_destroy(
    _FunnelHashTable * ht
) {
    if (ht->levels)
    {
        for (qo_int32_t i = 0; i < ht->num_levels; i++)   // Iterate only up to num_levels actually
                                                          // created
        {
            if (ht->levels[i])
            {
                // Free strings within the level's entries
                qo_int32_t  entry_count = ht->level_bucket_counts[i] * ht->beta;
                for (qo_int32_t j = 0; j < entry_count; ++j)
                {
                    if (ht->levels[i][j].occupied && ht->levels[i][j].key)
                    {
                        mi_free(ht->levels[i][j].key);
                    }
                }
                mi_free(ht->levels[i]); // Free the entry array for the level
            }
        }
        mi_free(ht->levels); // Free the array of level pointers
        ht->levels = NULL;
    }
    // Free other level metadata arrays
    mi_free(ht->level_bucket_counts);
    ht->level_bucket_counts = NULL;
    mi_free(ht->level_salts);
    ht->level_salts = NULL;
    mi_free(ht->level_occupancy);
    ht->level_occupancy = NULL;
    ht->num_levels = 0; // Reset level count
}

static qo_stat_t
funnel_levels_init(
    _FunnelHashTable * ht
) {
    // Calculation based on original logic
    qo_int32_t  primary_size = ht->capacity - ht->special_size;
    if (primary_size <= 0)
    {
        ht->num_levels = 0;
        ht->levels = NULL;
        ht->level_bucket_counts = NULL;
        ht->level_salts = NULL;
        ht->level_occupancy = NULL;
        return QO_OK;  // Valid state, only special array will be used
    }

    if (ht->beta <= 0)
    {
        ht->beta = 1;
    }   // Safe clamp

    qo_int32_t  total_buckets = (qo_int32_t) (primary_size /
        (ht->beta * 0.85f));
    // Slightly adjust load target if needed
    if (total_buckets <= 0)
    {
        total_buckets = 1;
    }
    qo_fp32_t  a1_denom = (1.0f - powf(0.8f , ht->alpha));
    qo_fp32_t  a1 = (ht->alpha > 0 &&
        fabsf(a1_denom) >
        1e-6) ? total_buckets / (3.5f * a1_denom) : (qo_fp32_t) total_buckets;

    if (ht->alpha <= 0)
    {
        ht->num_levels = 0;
        return QO_OK;
    }

    ht->levels = mi_malloc(ht->alpha * sizeof(_Entry *));
    ht->level_bucket_counts = mi_malloc(ht->alpha * sizeof(qo_int32_t));
    ht->level_salts = mi_malloc(ht->alpha * sizeof(qo_uint32_t));
    ht->level_occupancy = mi_calloc(ht->alpha , sizeof(qo_int32_t));

    if (!ht->levels || !ht->level_bucket_counts || !ht->level_salts ||
        !ht->level_occupancy)
    {
        funnel_levels_destroy(ht); // cleanup will handle partially allocated arrays
        return QO_OUT_OF_MEMORY;
    }

    memset(ht->levels , 0 , ht->alpha * sizeof(_Entry *));
    memset(ht->level_bucket_counts , 0 , ht->alpha * sizeof(qo_int32_t));
    memset(ht->level_salts , 0 , ht->alpha * sizeof(qo_uint32_t));

    for (qo_int32_t i = 0; i < ht->alpha; i++)
    {
        char  level_id_str[32];
        snprintf(level_id_str , sizeof(level_id_str) , "Level %d" , i);
        qo_uint64_t  level_hash64 = funnel_sz_hash_initial(level_id_str ,
            ht->special_salt);
        ht->level_salts[i] = (qo_uint32_t) (level_hash64 ^ (level_hash64 >>
            32));
        ht->level_salts[i] ^= (0x9e3779b9 + i);
    }

// Allocate actual level tables
    qo_int32_t  remaining_buckets = total_buckets;
    ht->num_levels = 0; // Count actual levels allocated
    for (qo_int32_t i = 0; i < ht->alpha; i++)
    {
        if (remaining_buckets <= 0)
        {
            break;                         // Stop if no more buckets needed
        }
        qo_int32_t  a_i = (qo_uint32_t) roundf(a1 * powf(0.8f , (qo_fp32_t) i));
        a_i = fmaxf(1 , a_i); // Min 1 bucket per level planned
        a_i = fminf(a_i , remaining_buckets); // Don't allocate more than remaining

        qo_int32_t  entries_needed = a_i * ht->beta;
        if (entries_needed <= 0)
        {
            // fprintf(stderr ,
            //     "Warning: Skipping level %d due to zero calculated entries.\n" ,
            //     i);
            continue;  // Skip level if calculation results in zero entries
        }

        ht->levels[i] = calloc(entries_needed , sizeof(_Entry));
        if (!ht->levels[i])
        {
            funnel_levels_destroy(ht);
            return QO_OUT_OF_MEMORY;
        }

        ht->level_bucket_counts[i] = a_i;
        remaining_buckets -= a_i;
        ht->num_levels++; // Increment count of successfully allocated levels
    }
    return true;
}

qo_stat_t
funnel_hash_table_init(
    _FunnelHashTable * ht ,
    qo_size_t          capacity ,
    qo_fp32_t          delta
) {
    if (capacity <= 0 || delta <= 0.0f || delta >= 1.0f)
    {
        return QO_INVALID_ARG;
    }

    ht->capacity = capacity;
    ht->delta = delta;
    ht->max_inserts = (qo_int32_t) (capacity * (1.0f - delta * 0.75f));

    // Calculate alpha and beta, adding sanity checks/caps
    ht->alpha = (qo_int32_t) ceilf(3.5f * log2f(1.0f / delta) + 8.0f);
    ht->beta  = (qo_int32_t) ceilf(2.5f * log2f(1.0f / delta));
    ht->alpha = (ht->alpha < 1) ? 1 : (ht->alpha > 100 ? 100 : ht->alpha); // Cap alpha
    ht->beta  = (ht->beta < 1) ? 1 : (ht->beta > 1024 ? 1024 : ht->beta); // Cap beta

    // Use qo_fp32_t for intermediate calculation before casting
    ht->special_size = (qo_int32_t) fmaxf(32.0f ,
        delta * (qo_fp32_t) capacity * 0.75f);
    // Ensure special_size is at least 1 for probe limit calculation
    if (ht->special_size <= 0)
    {
        ht->special_size = 32;
    }
    ht->probe_limit = calculate_probe_limit(ht->special_size);

    if (ht->special_size > 0)
    {
        ht->special_array = mi_calloc_tp(_Entry , 1);
        if (!ht->special_array)
        {
            return QO_OUT_OF_MEMORY;
        }
    }
    else {
        ht->special_array = NULL;
    }

    char  salt_base_str[64];
    snprintf(salt_base_str , sizeof(salt_base_str) ,
        "FunnelSaltBase%p_T%" PRIdMAX , ht , time(NULL));
    // It is a shit that the size of time_t is long long in Windows but long in Linux
    // I have to use PRIdMAX

    if (!funnel_levels_init(ht))
    {
        mi_free(ht->special_array);
        ht->special_array = NULL;
        return QO_OUT_OF_MEMORY;
    }
    return QO_OK;
}

void
funnel_hash_table_destroy(
    _FunnelHashTable * ht
) {
    if (!ht)
    {
        return;
    }

    funnel_levels_destroy(ht); // Clean up levels and strings within them

    // Clean up special array and strings within it
    if (ht->special_array)
    {
        for (qo_int32_t i = 0; i < ht->special_size; i++)
        {
            if (ht->special_array[i].key)   // Check key ptr before freeing
            {
                mi_free(ht->special_array[i].key);
                ht->special_array[i].key = NULL;
            }
        }
        mi_free(ht->special_array);
        ht->special_array = NULL;
    }
}

qo_bool_t
funnel_hash_table_search(
    const _FunnelHashTable * ht ,
    qo_ccstring_t            key ,
    qo_uint32_t *            value
) {
    if (!ht || !key)
    {
        return false;               // Basic null checks
    }
    // Search levels
    for (qo_int32_t i = 0; i < ht->num_levels; i++)
    {
        // Check level validity before using its data
        if (!ht->levels[i] || ht->level_bucket_counts[i] <= 0)
        {
            continue;
        }

        qo_uint64_t  hash_val = funnel_sz_hash_initial(key ,
            ht->level_salts[i]);
        // Use temporary qo_uint64_t for count to avoid potential overflow issues with %
        qo_uint64_t  current_level_bucket_count =
            (qo_uint64_t) ht->level_bucket_counts[i];
        qo_uint64_t  bucket_index = hash_val % current_level_bucket_count;

        qo_int32_t   start = (qo_int32_t) bucket_index * ht->beta;
        qo_int32_t   end = start + ht->beta;
        // Ensure end does not exceed allocated size for this level
        qo_int32_t   level_capacity = ht->level_bucket_counts[i] * ht->beta;
        if (end > level_capacity)
        {
            end = level_capacity;                       // Clamp end index
        }
        for (qo_int32_t j = start; j < end; j++)
        {
            // Check occupied FIRST, then check key pointer, then strcmp
            if (ht->levels[i][j].occupied && ht->levels[i][j].key != NULL &&
                strcmp(ht->levels[i][j].key , key) == 0)
            {
                if (value)
                {
                    *value = ht->levels[i][j].value;         // Assign value only if pointer is
                                                             // valid
                }
                return true;
            }
        }
    }

    // Search special array
    if (!ht->special_array || ht->special_size <= 0)
    {
        return false;                                              // Check validity
    }
    qo_uint64_t  hash_val = funnel_sz_hash_initial(key , ht->special_salt);
    qo_uint64_t  step = funnel_sz_hash_step(key);
    qo_uint64_t  current_special_size = (qo_uint64_t) ht->special_size;

    for (qo_int32_t i = 0; i < ht->probe_limit; i++)
    {
        qo_uint64_t  index64 = (hash_val + (qo_uint64_t) i * step) %
                               current_special_size;
        qo_int32_t   index = (qo_int32_t) index64;

        // Check occupied first
        if (!ht->special_array[index].occupied)
        {
            return false; // Early exit if empty slot found (assumes no deletions)
        }
        // If occupied, check key
        if (ht->special_array[index].key != NULL &&
            strcmp(ht->special_array[index].key , key) == 0)
        {
            if (value)
            {
                *value = ht->special_array[index].value;
            }
            return true;
        }
    }
    return false; // Not found after probing
}

// Return: true if inserted, false if failed
qo_bool_t
funnel_insert(
    _FunnelHashTable * ht ,
    qo_cstring_t       key_to_insert ,
    qo_uint32_t        value
) {
    if (!ht || !key_to_insert)
    {
        return false;                        // Null checks
    }
    if (ht->num_inserts >= ht->max_inserts)
    {
        //printf("    Inside funnel_insert for [%s]: FAILED early (num_inserts %d >= max_inserts
        // %d)\n",
        //       key_to_insert, ht->num_inserts, ht->max_inserts);
        return false;  // Table considered full based on policy
    }

    qo_uint32_t  existing_value;
    qo_bool_t    already_exists = funnel_hash_table_search(ht , key_to_insert ,
        &existing_value);
    //printf("    Inside funnel_insert for [%s]: funnel_search found? %s\n",
    //       key_to_insert, already_exists ? "Yes" : "No");
    if (already_exists)
    {
        return false; // Key already present
    }

    // --- Try inserting into levels ---
    for (qo_int32_t i = 0; i < ht->num_levels; i++)
    {
        if (!ht->levels[i] || ht->level_bucket_counts[i] <= 0)
        {
            continue;
        }

        uint64_t  hash_val = funnel_sz_hash_initial(key_to_insert ,
            ht->level_salts[i]);
        uint64_t  current_level_bucket_count =
            (uint64_t) ht->level_bucket_counts[i];
        uint64_t  bucket_index = hash_val % current_level_bucket_count;

        qo_int32_t  start = (qo_int32_t) bucket_index * ht->beta;
        qo_int32_t  end = start + ht->beta;
        qo_int32_t  level_capacity = ht->level_bucket_counts[i] * ht->beta;
        if (end > level_capacity)
        {
            end = level_capacity;
        }

        for (qo_int32_t j = start; j < end; j++)
        {
            if (!ht->levels[i][j].occupied)
            {
                //printf("      -> Inserting into Level %d, Slot %d\n", i, j);
                ht->levels[i][j].key = key_to_insert; // Takes ownership
                ht->levels[i][j].value = value;
                ht->levels[i][j].occupied = 1;
                ht->level_occupancy[i]++;
                ht->num_inserts++;
                return true; // Newly inserted
            }
        }
        ht->collisions++; // Bucket was full
    }

    // --- Try inserting into special array ---
    if (!ht->special_array || ht->special_size <= 0)
    {
        //printf("    Inside funnel_insert for [%s]: FAILED (no valid special array)\n",
        // key_to_insert);
        return false;
    }

    uint64_t  hash_val = funnel_sz_hash_initial(key_to_insert ,
        ht->special_salt);
    uint64_t  step = funnel_sz_hash_step(key_to_insert);
    uint64_t  current_special_size = (uint64_t) ht->special_size;

    for (qo_int32_t i = 0; i < ht->probe_limit; i++)
    {
        uint64_t  index64 = (hash_val + (uint64_t) i * step) %
                            current_special_size;
        qo_int32_t  index = (qo_int32_t) index64;

        if (!ht->special_array[index].occupied)
        {
            //printf("      -> Inserting into Special Array, Index %d (Probe %d)\n", index, i);
            ht->special_array[index].key = key_to_insert; // Takes ownership
            ht->special_array[index].value = value;
            ht->special_array[index].occupied = 1;
            ht->special_occupancy++;
            ht->num_inserts++;
            return true; // Newly inserted
        }
    }

    //printf("    Inside funnel_insert for [%s]: FAILED (probe limit %d reached in special
    // array)\n",
    //       key_to_insert, ht->probe_limit);
    return false; // Insertion failed (probe limit reached)
}

qo_stat_t
grapheme_collector_new(
    QOR_GraphemeCollector ** p_collector ,
    qo_size_t                capacity ,
    qo_fp32_t                funnel_delta
) {
    QOR_GraphemeCollector * collector =
        mi_malloc(sizeof(QOR_GraphemeCollector));
    if (!collector)
    {
        return NULL;
    }

    qo_stat_t  ret;
    ret = funnel_hash_table_init(&collector->ht , capacity , funnel_delta);
    if (ret != QO_OK)
    {
        mi_free(collector);
        return NULL;
    }

    qo_size_t  list_capacity = (qo_size_t) fmax(32.0f ,
        (qo_fp32_t) collector->ht.max_inserts);
    ret = order_list_init(&collector->order_list , list_capacity);
    if (ret != QO_OK)
    {
        funnel_hash_table_destroy(&collector->ht);
        mi_free(collector);
        return NULL;
    }

    collector->last_error = U_ZERO_ERROR;
    *p_collector = collector;
    return QO_OK;
}

void
qor_grapheme_collector_delete(
    QOR_GraphemeCollector * collector
) {
    if (collector)
    {
        order_list_destroy(&collector->order_list);
        funnel_hash_table_destroy(&collector->ht);
        mi_free(collector);
    }
}

qo_stat_t
qor_sample_utf16_grapheme(
    QOR_GraphemeCollector * collector ,
    UChar const *           text ,
    qo_size_t               text_len
) {
    if (!collector || !text || !text_len)
    {
        return QO_INVALID_ARG;
    }

    qo_bool_t  overall_success = true;

    UErrorCode  status = U_ZERO_ERROR;
    UNormalizer2 const * nfc_normalizer = unorm2_getNFCInstance(&status);
    if (U_FAILURE(status))
    {
        collector->last_error = status;
        return QO_UNKNOWN_ERROR;
    }

    status = U_ZERO_ERROR;
    qo_int32_t  normalized_utf16_len =
        unorm2_normalize(nfc_normalizer , text , text_len , NULL , 0 , &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR)
    {
        collector->last_error = status;
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
        collector->last_error = U_MEMORY_ALLOCATION_ERROR;
        return QO_OUT_OF_MEMORY;
    }

    status = U_ZERO_ERROR;
    unorm2_normalize(nfc_normalizer , text , text_len ,
        normalized_utf16_buffer , normalized_utf16_len , &status);
    if (U_FAILURE(status))
    {
        collector->last_error = status;
        mi_free(normalized_utf16_buffer);
        return QO_UNKNOWN_ERROR;
    }

    status = U_ZERO_ERROR;
    UBreakIterator * break_iterator = ubrk_open(UBRK_CHARACTER , "" ,
        normalized_utf16_buffer , normalized_utf16_len , &status);
    if (U_FAILURE(status) || !break_iterator)
    {
        collector->last_error = status;
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
                collector->last_error = status;
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
                collector->last_error = U_MEMORY_ALLOCATION_ERROR;
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
                collector->last_error = status;
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
                collector->last_error = U_MEMORY_ALLOCATION_ERROR;
                mi_free(current_grapheme_utf8);
                current_grapheme_utf8 = NULL;
                break;
            }

            if (funnel_insert(&collector->ht , grapheme_copy_for_table ,
                ENTRY_VALUE_DUMMY))
            {
                qo_cstring_t  grapheme_copy_for_list =
                    mi_strdup(current_grapheme_utf8);
                if (!grapheme_copy_for_list)
                {
                    overall_success = false;
                    collector->last_error = U_MEMORY_ALLOCATION_ERROR;
                    mi_free(current_grapheme_utf8);
                    current_grapheme_utf8 = NULL;
                    break;
                }

                if (!order_list_add(&collector->order_list ,
                    grapheme_copy_for_list))
                {
                    overall_success = false;
                    collector->last_error = U_MEMORY_ALLOCATION_ERROR;
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
    QOR_GraphemeCollector * collector ,
    qo_ccstring_t           utf8_text ,
    qo_size_t               utf8_len
) {
    if (!collector || !utf8_text || !utf8_len)
    {
        return QO_INVALID_ARG;
    }
    collector->last_error = U_ZERO_ERROR;

    UErrorCode  status = U_ZERO_ERROR;
    UChar * utf16_buffer  = NULL;
    qo_int32_t  utf16_len = 0;

    // Get the buffer size needed for conversion to UTF-16
    u_strFromUTF8(NULL , 0 , &utf16_len , utf8_text , utf8_len , &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status))
    {
        collector->last_error = status;
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
        collector->last_error = U_MEMORY_ALLOCATION_ERROR;
        return QO_OUT_OF_MEMORY;
    }

    // Perform true conversion
    status = U_ZERO_ERROR;
    u_strFromUTF8(utf16_buffer , utf16_len + 1 , NULL , utf8_text , utf8_len ,
        &status);
    if (U_FAILURE(status))
    {
        collector->last_error = status;
        mi_free(utf16_buffer);
        return QO_UNKNOWN_ERROR;
    }

    // Notice: the converted string is NOT normalized
    // So we let qor_sample_utf16_grapheme do it
    qo_stat_t  ret = qor_sample_utf16_grapheme(collector , utf16_buffer ,
        utf16_len);
    mi_free(utf16_buffer);
    return ret;
}

qo_stat_t
qor_grapheme_collector_view(
    QOR_GraphemeCollector *     collector ,
    QOR_GraphemeCollectorView * view
) {
    view->graphemes = (qo_ccstring_t *) collector->order_list.items;
    view->count = collector->order_list.size;
    return QO_OK;
}
