#pragma once
#define __QOR_STRING_HASH_WVKIMAGE_MAP_SRC__

#include "funnel_hash_table.h"
#include "wrapped_vulkan_image.h"
#include <xxh3.h>

typedef XXH64_hash_t string_hash_t;

typedef qo_size_t wvkimage_index_t;
struct __ActiveImageIndexEntry
{
    qo_uint32_t         index;
    qo_uint32_t         generation;
} QO_PACKED;
typedef struct __ActiveImageIndexEntry _ActiveImageIndexEntry;
static_assert(sizeof(_ActiveImageIndexEntry) == sizeof(qo_pointer_t) , "");

struct __StringHash2WVkImageIndexMap
{
    _FunnelHashTable table;
};
typedef struct __StringHash2WVkImageIndexMap _StringHash2WVkImageIndexMap;


qo_stat_t
string_hash_wvkimage_index_map_init(
    _StringHash2WVkImageIndexMap *  self ,
    qo_uint32_t                    capacity
);

void
string_hash_wvkimage_index_map_destroy(
    _StringHash2WVkImageIndexMap *  self 
);

qo_bool_t
string_hash_wvkimage_index_map_set(
    _StringHash2WVkImageIndexMap *  self ,
    string_hash_t                   key ,
    _ActiveImageIndexEntry          index_entry
);

qo_bool_t
string_hash_wvkimage_index_map_search(
    _StringHash2WVkImageIndexMap *  self ,
    string_hash_t                   key ,
    _ActiveImageIndexEntry *        p_index_entry
);
