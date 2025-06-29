// --- START OF FILE: funnel_ht.c ---
#include "funnel_hash_table.h"

#include <mimalloc.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define FHT_EMPTY_VALUE  INTMAX_MIN

static void
cleanup_levels(
    _FunnelHashTable * self
) {
    if (!self || !self->levels)
    {
        return;
    }
    // self->num_levels tracks how many levels were successfully allocated.
    for (qo_int32_t i = 0; i < self->num_levels; i++)
    {
        if (self->levels[i])
        {
            mi_free(self->levels[i]);
        }
    }
    mi_free(self->levels);
    mi_free(self->level_bucket_counts);
    mi_free(self->level_salts);
    mi_free(self->level_occupancy);
    self->levels = NULL;
    self->level_bucket_counts = NULL;
    self->level_salts = NULL;
    self->level_occupancy = NULL;
}

static qo_size_t
calculate_probe_limit(
    qo_size_t  capacity
) {
    if (capacity < 2)
    {
        return 1;
    }
    return (qo_size_t) ceilf(logf((qo_fp32_t) capacity) * 3.0f);
}

static qo_stat_t
initialize_levels(
    _FunnelHashTable * self
) {
    qo_int32_t  primary_size = self->capacity - self->special_size;
    if (primary_size <= 0)
    {
        self->num_levels = 0;
        return QO_OK;
    }
    if (self->beta <= 0)
    {
        self->beta = 1;
    }
    qo_int32_t  total_buckets = (qo_int32_t) (primary_size /
        (self->beta * 0.85f));
    if (total_buckets <= 0)
    {
        total_buckets = 1;
    }
    qo_fp32_t  a1_denom = (1.0f - powf(0.8f , (qo_fp32_t) self->alpha));
    qo_fp32_t  a1 = (self->alpha > 0 &&
        fabsf(a1_denom) >
        1e-6) ? (qo_fp32_t) total_buckets /
                    (3.5f * a1_denom) : (qo_fp32_t) total_buckets;
    if (self->alpha <= 0)
    {
        self->num_levels = 0;
        return QO_OK;
    }
    self->levels = calloc((qo_size_t) self->alpha ,
        sizeof(_FunnelHashTableEntry *));
    self->level_bucket_counts = calloc((qo_size_t) self->alpha ,
        sizeof(qo_int32_t));
    self->level_salts = calloc((qo_size_t) self->alpha , sizeof(qo_uint32_t));
    self->level_occupancy = calloc((qo_size_t) self->alpha ,
        sizeof(qo_int32_t));
    if (!self->levels || !self->level_bucket_counts || !self->level_salts ||
        !self->level_occupancy)
    {
        cleanup_levels(self);
        return QO_OUT_OF_MEMORY;
    }
    for (qo_int32_t i = 0; i < self->alpha; i++)
    {
        self->level_salts[i] = self->special_salt ^ (0x9e3779b9 +
            (qo_uint32_t) i * 0x1000193);
    }
    qo_int32_t  remaining_buckets = total_buckets;
    self->num_levels = 0;
    for (qo_int32_t i = 0; i < self->alpha; i++)
    {
        if (remaining_buckets <= 0)
        {
            break;
        }
        qo_int32_t  a_i = (qo_int32_t) roundf(a1 * powf(0.8f , (qo_fp32_t) i));
        a_i = fmaxf(1 , a_i);
        a_i = fminf(a_i , remaining_buckets);
        qo_int32_t  entries_needed = a_i * self->beta;
        if (entries_needed <= 0)
        {
            continue;
        }
        self->levels[i] = mi_mallocn_tp(_FunnelHashTableEntry, entries_needed);

        if (!self->levels[i])
        {
            cleanup_levels(self);
            return QO_OUT_OF_MEMORY;
        }
        for (qo_int32_t j = 0; j < entries_needed; ++j)
        {
            self->levels[i][j].value = FHT_EMPTY_VALUE;
        }
        self->level_bucket_counts[i] = a_i;
        remaining_buckets -= a_i;
        self->num_levels++;
    }
    return QO_OK;
}

qo_stat_t
funnel_ht_init(
    _FunnelHashTable *          self ,
    qo_size_t                   capacity ,
    qo_fp32_t                   delta ,
    _FunnelHashTableAuxiliary * p_auxiliary
) {
    memset(self , 0 , sizeof(_FunnelHashTable));
    self->capacity = (qo_int32_t) capacity;
    self->delta = delta;
    self->auxiliary = *p_auxiliary;
    self->max_inserts = (qo_int32_t) (capacity * (1.0f - delta * 0.75f));
    self->alpha = (qo_int32_t) ceilf(3.5f * log2f(1.0f / delta) + 8.0f);
    self->beta  = (qo_int32_t) ceilf(2.5f * log2f(1.0f / delta));
    self->alpha = (self->alpha < 1) ? 1 : (self->alpha >
        100 ? 100 : self->alpha);
    self->beta = (self->beta < 1) ? 1 : (self->beta > 1024 ? 1024 : self->beta);
    self->special_size = (qo_int32_t) fmaxf(32.0f ,
        delta * (qo_fp32_t) capacity * 0.75f);
    if (self->special_size <= 0)
    {
        self->special_size = 32;
    }
    self->probe_limit = (qo_int32_t) calculate_probe_limit(self->special_size);
    if (self->special_size > 0)
    {
        self->special_array = mi_mallocn_tp(_FunnelHashTableEntry ,
            self->special_size);
        if (!self->special_array)
        {
            return QO_OUT_OF_MEMORY;
        }
        for (qo_int32_t i = 0; i < self->special_size; ++i)
        {
            self->special_array[i].value = FHT_EMPTY_VALUE;
        }
    }
    self->special_salt = (qo_uint32_t) time(NULL) ^
                         (qo_uint32_t) (intptr_t) self;
    if (initialize_levels(self) != QO_OK)
    {
        mi_free(self->special_array);
        return QO_OUT_OF_MEMORY;
    }
    return QO_OK;
}

_FunnelHashTable *
fht_new(
    qo_size_t                   capacity ,
    qo_fp32_t                   delta ,
    _FunnelHashTableAuxiliary * p_auxiliary
) {
    _FunnelHashTable * self = mi_malloc_tp(_FunnelHashTable);
    if (!self)
    {
        return NULL;
    }
    if (funnel_ht_init(self , capacity , delta , p_auxiliary) != QO_OK)
    {
        mi_free(self);
        return NULL;
    }
    return self;
}

void
funnel_ht_destroy(
    _FunnelHashTable * self
) {
    if (!self)
    {
        return;
    }
    if (self->auxiliary.key_destroy_func)
    {
        fht_key_t  key;
        fht_value_t  value;
        _FunnelHashTableIterator  iterator = fht_iterate(self);
        while (fht_iterator_next(&iterator , &key , &value))
        {
            self->auxiliary.key_destroy_func(key);
        }
    }
    cleanup_levels(self);
    mi_free(self->special_array);
}

_FunnelHashTableEntry *
fht_find_slot(
    _FunnelHashTable *  self,
    fht_key_t           key ,
    qo_bool_t           find_empty_for_insert
) {
    // Stage 1: Search for an existing key or the first empty slow in a bucket
    for (qo_int32_t i = 0 ; i < self->num_levels ; i++)
    {
        qo_uint64_t hash_value = self->auxiliary.hash_func(key , self->level_salts[i]);
        qo_int32_t  bucket_index = (qo_int32_t) (hash_value % (qo_uint64_t) self->level_bucket_counts[i]);
        qo_int32_t  start = bucket_index * self->beta;
        qo_int32_t  end = start + self->beta;
        _FunnelHashTableEntry * first_empty_in_bucket = NULL;

        for (qo_int32_t j = start ; j < end ; j++)
        {
            _FunnelHashTableEntry * entry = &self->levels[i][j];
            if (entry->value == FHT_EMPTY_VALUE)
            {
                if (!first_empty_in_bucket &&  find_empty_for_insert)
                { // Found a potential insertion spot
                    first_empty_in_bucket = entry; 
                }
            }
            else if (self->auxiliary.equals_func(entry->key , key))
            { // Found extsing key, return its slot
                return entry;
            }
        }
        if (first_empty_in_bucket && find_empty_for_insert)
        {
            return first_empty_in_bucket;
        }
        // self->collisions++;
    } 

    // Stage 2: Probe the special array
    if (!self->special_array)
        return NULL;

    qo_uint64_t hash_value = self->auxiliary.hash_func(key , self->special_salt);
    qo_uint64_t step = self->auxiliary.hash_func(key , ~self->special_salt) | 1;

    for(qo_int32_t i = 0; i <self->probe_limit ; i++)
    {
        qo_int32_t index = ((hash_value + (qo_uint64_t)i * step) % (qo_uint64_t)self->special_size);
        _FunnelHashTableEntry * entry = &self->special_array[index];
        if (entry->value == FHT_EMPTY_VALUE)
        {
            return entry;
        }
        if (self->auxiliary.equals_func(entry->key , key))
        {
            return entry;
        }
    }
    return NULL; // Table is full
}

qo_bool_t
fht_set(
    _FunnelHashTable *  self ,
    fht_key_t           key ,
    fht_value_t         value
) {
    if (value == FHT_EMPTY_VALUE)
        return qo_false;

    _FunnelHashTableEntry * slot = fht_find_slot(self, key , qo_true);
    if (!slot)
    {
        return qo_false;
    }
    if (slot->value != FHT_EMPTY_VALUE)
    { // Update Path
        if (self->auxiliary.key_destroy_func)
        {
            self->auxiliary.key_destroy_func(slot->key);
        }
        slot->key = key;
        slot->value = value;
    }
    else 
    { // Insert Path
        if (self->num_inserts >= self->max_inserts)
        {
            return qo_false;
        }
        slot->key = key;
        slot->value = value;
        self->num_inserts++; 
        // TODO: update level_occpancy 
    }
    return qo_true;
}

qo_bool_t
fht_search(
    _FunnelHashTable const * self ,
    fht_key_t                key ,
    fht_value_t *            p_value
) {
    _FunnelHashTableEntry * entry = fht_find_slot((_FunnelHashTable *)self, key, qo_false);
    if (entry)
    {
        if (p_value)
        {
            *p_value = entry->value;
        }
        return qo_true;
    }
    return qo_false;
}

qo_bool_t
funnel_ht_insert(
    _FunnelHashTable * self ,
    fht_key_t          key ,
    fht_value_t        value
) {
    if (self->num_inserts >= self->max_inserts || value == FHT_EMPTY_VALUE)
    {
        return qo_false;
    }

    _FunnelHashTableEntry * slot= fht_find_slot(self, key , qo_true);
    if (!slot || slot->value != FHT_EMPTY_VALUE)
    {
        return qo_false;
    }
    slot->key = key;
    slot->value = value;
    self->num_inserts++;
    return qo_true;
}

_FunnelHashTableIterator
fht_iterate(
    _FunnelHashTable const * ht
) {
    _FunnelHashTableIterator  iterator = { .ht = ht , .level_index = 0 ,
                                           .entry_index = -1 };
    return iterator;
}

qo_bool_t
fht_iterator_next(
    _FunnelHashTableIterator * iterator ,
    fht_key_t *                p_key ,
    fht_value_t *              p_value
) {
    if (!iterator || !iterator->ht)
    {
        return qo_false;
    }
    _FunnelHashTable const * ht = iterator->ht;
    while (qo_true)
    {
        iterator->entry_index++;
        if (iterator->level_index < ht->num_levels)
        {
            qo_int32_t  level_capacity =
                ht->level_bucket_counts[iterator->level_index] * ht->beta;
            if (iterator->entry_index < level_capacity)
            {
                _FunnelHashTableEntry * entry =
                    &ht->levels[iterator->level_index][iterator->entry_index];
                if (entry->value != FHT_EMPTY_VALUE)
                {
                    if (p_key)
                    {
                        *p_key = entry->key;
                    }
                    if (p_value)
                    {
                        *p_value = entry->value;
                    }
                    return qo_true;
                }
                continue;
            }
            iterator->level_index++;
            iterator->entry_index = -1;
            continue;
        }
        else if (iterator->level_index == ht->num_levels)
        {
            if (!ht->special_array)
            {
                iterator->level_index++;
                return qo_false;
            }
            if (iterator->entry_index < ht->special_size)
            {
                _FunnelHashTableEntry * entry =
                    &ht->special_array[iterator->entry_index];
                if (entry->value != FHT_EMPTY_VALUE)
                {
                    if (p_key)
                    {
                        *p_key = entry->key;
                    }
                    if (p_value)
                    {
                        *p_value = entry->value;
                    }
                    return qo_true;
                }
                continue;
            }
            iterator->level_index++;
            return qo_false;
        }
        else {
            return qo_false;
        }
    }
}
