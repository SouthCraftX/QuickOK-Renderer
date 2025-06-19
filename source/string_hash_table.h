#include "../include/renderer.h"

struct __OrderList
{
    qo_cstring_t * items;
    qo_size_t      size;
    qo_size_t      capacity;
};
typedef struct __OrderList _OrderList;

#define ENTRY_VALUE_DUMMY  0

struct __Entry
{
    qo_cstring_t  key;
    qo_bool_t     occupied;
    qo_uint32_t   value; // reserved
};
typedef struct __Entry _Entry;

struct __FunnelHashSet
{
    // Core parameters
    qo_int32_t  capacity;
    qo_int32_t  max_inserts; // Max unique items based on capacity/delta
    qo_int32_t  num_inserts; // Current unique items inserted
    qo_fp32_t   delta;

    // Level parameters
    qo_int32_t    alpha; // Num levels
    qo_int32_t    beta; // Bucket size
    _Entry **     levels; // Array of level tables
    qo_int32_t *  level_bucket_counts; // Buckets per level
    qo_uint32_t * level_salts; // Salts for hashing at each level
    qo_int32_t    num_levels; // Actual number of levels created

    // Special array
    _Entry *     special_array;
    qo_int32_t   special_size;
    qo_uint32_t  special_salt;
    qo_int32_t   special_occupancy;
    qo_int32_t   probe_limit;

    // Diagnostics
    qo_int32_t * level_occupancy; // Items per level
    qo_int32_t   collisions;   // Bucket full events (before trying special array)
    qo_int32_t   value_errors; // Not really applicable here, maybe repurpose?
};
typedef struct __FunnelHashSet _FunnelHashTable;
qo_stat_t
funnel_hash_table_init(
    _FunnelHashTable * self ,
    qo_size_t          capacity ,
    qo_fp32_t          delta
);

void
funnel_hash_table_destroy(
    _FunnelHashTable * self
);

qo_bool_t
funnel_hash_table_search(
    _FunnelHashTable const * self ,
    qo_ccstring_t             key ,
    qo_uint32_t *            p_value
);

qo_bool_t
funnel_hash_table_insert(
    _FunnelHashTable * self ,
    qo_cstring_t       key ,
    qo_uint32_t        value
);

qo_stat_t
order_list_init(
    _OrderList * self ,
    qo_size_t    initial_capacity
);

void
order_list_destroy(
    _OrderList * self
);

qo_bool_t
order_list_add(
    _OrderList *  self ,
    qo_cstring_t  item
);
