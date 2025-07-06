#pragma once
#define __QOR_FUNNEL_HASH_TABLE__

#include "../rendering_env.h"
#include <limits.h>
#include <xxh3.h>

#define FHT_EMPTY_VALUE  INTMAX_MIN

typedef qo_intmax_t fht_key_t;
typedef qo_intmax_t fht_value_t;
typedef XXH64_hash_t string_hash_t;

typedef 
    qo_size_t 
    (* fht_hash_f)(
        fht_key_t   key ,
        qo_uint32_t salt
    );

typedef 
    qo_bool_t 
    (* fht_equals_f)(
        fht_key_t   key1 ,
        fht_key_t   key2
    );

typedef 
    void 
    (* fht_key_destroy_f)(
        fht_key_t   key
    );

struct __FunnelHashTableAuxiliary
{
    fht_hash_f          hash_func;
    fht_equals_f        equals_func;
    fht_key_destroy_f   key_destroy_func;
};
typedef struct __FunnelHashTableAuxiliary _FunnelHashTableAuxiliary;

struct __FunnelHashTableEntry
{
    fht_key_t    key;
    fht_value_t  value;
};
typedef struct __FunnelHashTableEntry _FunnelHashTableEntry;

struct __FunnelHashTable
{
    qo_int32_t                 capacity;
    qo_int32_t                 max_inserts;
    qo_int32_t                 num_inserts;
    qo_fp32_t                  delta;
    qo_int32_t                 alpha;
    qo_int32_t                 beta;
    _FunnelHashTableEntry **   levels;
    qo_int32_t *               level_bucket_counts;
    qo_uint32_t *              level_salts;
    qo_int32_t                 num_levels;
    _FunnelHashTableEntry *    special_array;
    qo_int32_t                 special_size;
    qo_uint32_t                special_salt;
    qo_int32_t                 special_occupancy;
    qo_int32_t                 probe_limit;
    qo_int32_t *               level_occupancy;
    qo_int32_t                 collisions;
    _FunnelHashTableAuxiliary  auxiliary;
};
typedef struct __FunnelHashTable _FunnelHashTable;

typedef struct __FunnelHashTableIterator _FunnelHashTableIterator;
struct __FunnelHashTableIterator
{
    _FunnelHashTable const * ht;
    qo_int32_t               level_index;
    qo_int32_t               entry_index;
};

qo_stat_t
fht_init(
    _FunnelHashTable *          self ,
    qo_size_t                   capacity ,
    qo_fp32_t                   delta ,
    _FunnelHashTableAuxiliary * p_auxiliary
);

_FunnelHashTableEntry *
fht_find_slot(
    _FunnelHashTable *  self,
    fht_key_t           key ,
    qo_bool_t           find_empty_for_insert
);

void
fht_destroy(
    _FunnelHashTable *          self
);

qo_bool_t
fht_search(
    _FunnelHashTable const *    self ,
    fht_key_t                   key ,
    fht_value_t *               p_value
);

qo_bool_t
fht_insert(
    _FunnelHashTable *          self ,
    fht_key_t                   key ,
    fht_value_t                 value
);

qo_bool_t
fht_set(
    _FunnelHashTable *          self ,
    fht_key_t                   key ,
    fht_value_t                 value
);

qo_size_t
fht_get_count(
    _FunnelHashTable const *    self
);

_FunnelHashTableIterator 
fht_iterate(
    _FunnelHashTable const *    self
);

qo_bool_t
fht_iterator_next(
    _FunnelHashTableIterator *  self ,
    fht_key_t *                 p_key ,
    fht_value_t *               p_value
);

