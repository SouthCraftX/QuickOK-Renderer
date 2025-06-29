#include "../../QuickOK-Zero/include/stringzilla.h" // TODO: remove
#include "string_hash_table.h"
#include <math.h>
#include <mimalloc.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
// returns true if failed
qo_stat_t
order_list_init(
    _OrderList * self ,
    qo_size_t    initial_capacity
) {
    initial_capacity = initial_capacity ? initial_capacity : 8;
    self->items = (qo_cstring_t *) mi_malloc(
        initial_capacity * sizeof(qo_cstring_t)
    );

    if (self->items)
    {
        self->size = 0;
        self->capacity = initial_capacity;
        return QO_OK;
    }
    return QO_OUT_OF_MEMORY;
}

void
order_list_destroy(
    _OrderList * self
) {
    for (qo_size_t i = 0; i < self->size; i++)
    {
        mi_free(self->items[i]);
    }
    mi_free(self->items);
}

qo_bool_t
// returns true if failed
order_list_add(
    _OrderList *  self ,
    qo_cstring_t  item
) {
    if (self->size == self->capacity)
    {
        qo_size_t  new_capacity = self->capacity ? self->capacity * 2 : 8;
        qo_cstring_t * new_items = mi_reallocn_tp(self->items, qo_cstring_t, new_capacity);

        if (!new_items)
        {
            return qo_true;
        }
        self->items = new_items;
        self->capacity = new_capacity;
    }
    self->items[self->size++] = item; // take ownership of item
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
    _FunnelHashTable * self
) {
    if (self->levels)
    {
        for (qo_int32_t i = 0; i < self->num_levels; i++)   // Iterate only up to num_levels actually
                                                          // created
        {
            if (self->levels[i])
            {
                // Free strings within the level's entries
                qo_int32_t  entry_count = self->level_bucket_counts[i] * self->beta;
                for (qo_int32_t j = 0; j < entry_count; ++j)
                {
                    if (self->levels[i][j].occupied && self->levels[i][j].key)
                    {
                        mi_free(self->levels[i][j].key);
                    }
                }
                mi_free(self->levels[i]); // Free the entry array for the level
            }
        }
        mi_free(self->levels); // Free the array of level pointers
        self->levels = NULL;
    }
    // Free other level metadata arrays
    mi_free(self->level_bucket_counts);
    self->level_bucket_counts = NULL;
    mi_free(self->level_salts);
    self->level_salts = NULL;
    mi_free(self->level_occupancy);
    self->level_occupancy = NULL;
    self->num_levels = 0; // Reset level count
}

static qo_stat_t
funnel_levels_init(
    _FunnelHashTable * self
) {
    // Calculation based on original logic
    qo_int32_t  primary_size = self->capacity - self->special_size;
    if (primary_size <= 0)
    {
        self->num_levels = 0;
        self->levels = NULL;
        self->level_bucket_counts = NULL;
        self->level_salts = NULL;
        self->level_occupancy = NULL;
        return QO_OK;  // Valid state, only special array will be used
    }

    if (self->beta <= 0)
    {
        self->beta = 1;
    }   // Safe clamp

    qo_int32_t  total_buckets = (qo_int32_t) (primary_size /
        (self->beta * 0.85f));
    // Slightly adjust load target if needed
    if (total_buckets <= 0)
    {
        total_buckets = 1;
    }
    qo_fp32_t  a1_denom = (1.0f - powf(0.8f , self->alpha));
    qo_fp32_t  a1 = (self->alpha > 0 &&
        fabsf(a1_denom) >
        1e-6) ? total_buckets / (3.5f * a1_denom) : (qo_fp32_t) total_buckets;

    if (self->alpha <= 0)
    {
        self->num_levels = 0;
        return QO_OK;
    }

    self->levels = mi_malloc(self->alpha * sizeof(_Entry *));
    self->level_bucket_counts = mi_malloc(self->alpha * sizeof(qo_int32_t));
    self->level_salts = mi_malloc(self->alpha * sizeof(qo_uint32_t));
    self->level_occupancy = mi_calloc(self->alpha , sizeof(qo_int32_t));

    if (!self->levels || !self->level_bucket_counts || !self->level_salts ||
        !self->level_occupancy)
    {
        funnel_levels_destroy(self); // cleanup will handle partially allocated arrays
        return QO_OUT_OF_MEMORY;
    }

    memset(self->levels , 0 , self->alpha * sizeof(_Entry *));
    memset(self->level_bucket_counts , 0 , self->alpha * sizeof(qo_int32_t));
    memset(self->level_salts , 0 , self->alpha * sizeof(qo_uint32_t));

    for (qo_int32_t i = 0; i < self->alpha; i++)
    {
        char  level_id_str[32];
        snprintf(level_id_str , sizeof(level_id_str) , "Level %d" , i);
        qo_uint64_t  level_hash64 = funnel_sz_hash_initial(level_id_str ,
            self->special_salt);
        self->level_salts[i] = (qo_uint32_t) (level_hash64 ^ (level_hash64 >>
            32));
        self->level_salts[i] ^= (0x9e3779b9 + i);
    }

// Allocate actual level tables
    qo_int32_t  remaining_buckets = total_buckets;
    self->num_levels = 0; // Count actual levels allocated
    for (qo_int32_t i = 0; i < self->alpha; i++)
    {
        if (remaining_buckets <= 0)
        {
            break;                         // Stop if no more buckets needed
        }
        qo_int32_t  a_i = (qo_uint32_t) roundf(a1 * powf(0.8f , (qo_fp32_t) i));
        a_i = fmaxf(1 , a_i); // Min 1 bucket per level planned
        a_i = fminf(a_i , remaining_buckets); // Don't allocate more than remaining

        qo_int32_t  entries_needed = a_i * self->beta;
        if (entries_needed <= 0)
        {
            // fprintf(stderr ,
            //     "Warning: Skipping level %d due to zero calculated entries.\n" ,
            //     i);
            continue;  // Skip level if calculation results in zero entries
        }

        self->levels[i] = calloc(entries_needed , sizeof(_Entry));
        if (!self->levels[i])
        {
            funnel_levels_destroy(self);
            return QO_OUT_OF_MEMORY;
        }

        self->level_bucket_counts[i] = a_i;
        remaining_buckets -= a_i;
        self->num_levels++; // Increment count of successfully allocated levels
    }
    return true;
}

qo_stat_t
funnel_hash_table_init(
    _FunnelHashTable * self ,
    qo_size_t          capacity ,
    qo_fp32_t          delta
) {
    if (capacity <= 0 || delta <= 0.0f || delta >= 1.0f)
    {
        return QO_INVALID_ARG;
    }

    self->capacity = capacity;
    self->delta = delta;
    self->max_inserts = (qo_int32_t) (capacity * (1.0f - delta * 0.75f));

    // Calculate alpha and beta, adding sanity checks/caps
    self->alpha = (qo_int32_t) ceilf(3.5f * log2f(1.0f / delta) + 8.0f);
    self->beta  = (qo_int32_t) ceilf(2.5f * log2f(1.0f / delta));
    self->alpha = (self->alpha < 1) ? 1 : (self->alpha > 100 ? 100 : self->alpha); // Cap alpha
    self->beta  = (self->beta < 1) ? 1 : (self->beta > 1024 ? 1024 : self->beta); // Cap beta

    // Use qo_fp32_t for intermediate calculation before casting
    self->special_size = (qo_int32_t) fmaxf(32.0f ,
        delta * (qo_fp32_t) capacity * 0.75f);
    // Ensure special_size is at least 1 for probe limit calculation
    if (self->special_size <= 0)
    {
        self->special_size = 32;
    }
    self->probe_limit = calculate_probe_limit(self->special_size);

    if (self->special_size > 0)
    {
        self->special_array = mi_calloc_tp(_Entry , 1);
        if (!self->special_array)
        {
            return QO_OUT_OF_MEMORY;
        }
    }
    else {
        self->special_array = NULL;
    }

    char  salt_base_str[64];
    snprintf(salt_base_str , sizeof(salt_base_str) ,
        "FunnelSaltBase%p_T%" PRIdMAX , self , time(NULL));
    // It is a shit that the size of time_t is long long in Windows but long in Linux
    // I have to use PRIdMAX

    if (!funnel_levels_init(self))
    {
        mi_free(self->special_array);
        self->special_array = NULL;
        return QO_OUT_OF_MEMORY;
    }
    return QO_OK;
}

void
funnel_hash_table_destroy(
    _FunnelHashTable * self
) {
    if (!self)
    {
        return;
    }

    funnel_levels_destroy(self); // Clean up levels and strings within them

    // Clean up special array and strings within it
    if (self->special_array)
    {
        for (qo_int32_t i = 0; i < self->special_size; i++)
        {
            if (self->special_array[i].key)   // Check key ptr before freeing
            {
                mi_free(self->special_array[i].key);
                self->special_array[i].key = NULL;
            }
        }
        mi_free(self->special_array);
        self->special_array = NULL;
    }
}

qo_bool_t
funnel_hash_table_search(
    _FunnelHashTable const * self ,
    qo_ccstring_t            key ,
    qo_uint32_t *            p_value
) {
    if (!self || !key)
    {
        return false;               // Basic null checks
    }
    // Search levels
    for (qo_int32_t i = 0; i < self->num_levels; i++)
    {
        // Check level validity before using its data
        if (!self->levels[i] || self->level_bucket_counts[i] <= 0)
        {
            continue;
        }

        qo_uint64_t  hash_val = funnel_sz_hash_initial(key ,
            self->level_salts[i]);
        // Use temporary qo_uint64_t for count to avoid potential overflow issues with %
        qo_uint64_t  current_level_bucket_count =
            (qo_uint64_t) self->level_bucket_counts[i];
        qo_uint64_t  bucket_index = hash_val % current_level_bucket_count;

        qo_int32_t   start = (qo_int32_t) bucket_index * self->beta;
        qo_int32_t   end = start + self->beta;
        // Ensure end does not exceed allocated size for this level
        qo_int32_t   level_capacity = self->level_bucket_counts[i] * self->beta;
        if (end > level_capacity)
        {
            end = level_capacity;                       // Clamp end index
        }
        for (qo_int32_t j = start; j < end; j++)
        {
            // Check occupied FIRST, then check key pointer, then strcmp
            if (self->levels[i][j].occupied && self->levels[i][j].key != NULL &&
                strcmp(self->levels[i][j].key , key) == 0)
            {
                if (p_value)
                {
                    *p_value = self->levels[i][j].value;         
                    // Assign value only if pointer is valid
                }
                return true;
            }
        }
    }

    // Search special array
    if (!self->special_array || self->special_size <= 0)
    {
        return false;   // Check validity
    }
    qo_uint64_t  hash_val = funnel_sz_hash_initial(key , self->special_salt);
    qo_uint64_t  step = funnel_sz_hash_step(key);
    qo_uint64_t  current_special_size = (qo_uint64_t) self->special_size;

    for (qo_int32_t i = 0; i < self->probe_limit; i++)
    {
        qo_uint64_t  index64 = (hash_val + (qo_uint64_t) i * step) %
                               current_special_size;
        qo_int32_t   index = (qo_int32_t) index64;

        // Check occupied first
        if (!self->special_array[index].occupied)
        {
            return false; // Early exit if empty slot found (assumes no deletions)
        }
        // If occupied, check key
        if (self->special_array[index].key != NULL &&
            strcmp(self->special_array[index].key , key) == 0)
        {
            if (p_value)
            {
                *p_value = self->special_array[index].value;
            }
            return true;
        }
    }
    return false; // Not found after probing
}

// Return: true if inserted, false if failed
qo_bool_t
funnel_hash_table_insert(
    _FunnelHashTable * self ,
    qo_cstring_t       key_to_insert ,
    qo_uint32_t        value
) {
    if (!self || !key_to_insert)
    {
        return false;                        // Null checks
    }
    if (self->num_inserts >= self->max_inserts)
    {
        //printf("    Inside funnel_insert for [%s]: FAILED early (num_inserts %d >= max_inserts
        // %d)\n",
        //       key_to_insert, ht->num_inserts, ht->max_inserts);
        return false;  // Table considered full based on policy
    }

    qo_uint32_t  existing_value;
    qo_bool_t    already_exists = funnel_hash_table_search(self , key_to_insert ,
        &existing_value);
    //printf("    Inside funnel_insert for [%s]: funnel_search found? %s\n",
    //       key_to_insert, already_exists ? "Yes" : "No");
    if (already_exists)
    {
        return false; // Key already present
    }

    // --- Try inserting into levels ---
    for (qo_int32_t i = 0; i < self->num_levels; i++)
    {
        if (!self->levels[i] || self->level_bucket_counts[i] <= 0)
        {
            continue;
        }

        uint64_t  hash_val = funnel_sz_hash_initial(key_to_insert ,
            self->level_salts[i]);
        uint64_t  current_level_bucket_count =
            (uint64_t) self->level_bucket_counts[i];
        uint64_t  bucket_index = hash_val % current_level_bucket_count;

        qo_int32_t  start = (qo_int32_t) bucket_index * self->beta;
        qo_int32_t  end = start + self->beta;
        qo_int32_t  level_capacity = self->level_bucket_counts[i] * self->beta;
        if (end > level_capacity)
        {
            end = level_capacity;
        }

        for (qo_int32_t j = start; j < end; j++)
        {
            if (!self->levels[i][j].occupied)
            {
                //printf("      -> Inserting into Level %d, Slot %d\n", i, j);
                self->levels[i][j].key = key_to_insert; // Takes ownership
                self->levels[i][j].value = value;
                self->levels[i][j].occupied = 1;
                self->level_occupancy[i]++;
                self->num_inserts++;
                return true; // Newly inserted
            }
        }
        self->collisions++; // Bucket was full
    }

    // --- Try inserting into special array ---
    if (!self->special_array || self->special_size <= 0)
    {
        //printf("    Inside funnel_insert for [%s]: FAILED (no valid special array)\n",
        // key_to_insert);
        return false;
    }

    uint64_t  hash_val = funnel_sz_hash_initial(key_to_insert ,
        self->special_salt);
    uint64_t  step = funnel_sz_hash_step(key_to_insert);
    uint64_t  current_special_size = (uint64_t) self->special_size;

    for (qo_int32_t i = 0; i < self->probe_limit; i++)
    {
        uint64_t  index64 = (hash_val + (uint64_t) i * step) %
                            current_special_size;
        qo_int32_t  index = (qo_int32_t) index64;

        if (!self->special_array[index].occupied)
        {
            //printf("      -> Inserting into Special Array, Index %d (Probe %d)\n", index, i);
            self->special_array[index].key = key_to_insert; // Takes ownership
            self->special_array[index].value = value;
            self->special_array[index].occupied = 1;
            self->special_occupancy++;
            self->num_inserts++;
            return true; // Newly inserted
        }
    }

    //printf("    Inside funnel_insert for [%s]: FAILED (probe limit %d reached in special
    // array)\n",
    //       key_to_insert, ht->probe_limit);
    return false; // Insertion failed (probe limit reached)
}