#pragma once
#define __QOR_VKIMAGE_VIEW_HASH_WVKIMAGE_LIST_MAP_SRC__
#include "../container/funnel_hash_table.h"
#include "wrapped_vulkan_image.h"
#include "wrapped_vulkan_image_list.h"

struct __VkImageInfoCompositeKey
{
    XXH64_hash_t            precomputed_hash;
    VkImageCreateInfo *     p_info;
};
typedef struct __VkImageInfoCompositeKey _VkImageInfoCompositeKey;

struct __VkImageInfoHash2WVkImageListMap
{
    _FunnelHashTable  hash_table;
};
typedef struct __VkImageInfoHash2WVkImageListMap
        _VkImageInfoHash2WVkImageListMap;

struct __VkImagInfoHash2WVkImageListMapIterator
{
    _FunnelHashTableIterator fht_iter;
};
typedef struct __VkImagInfoHash2WVkImageListMapIterator
        _VkImagInfoHash2WVkImageListMapIterator;

qo_stat_t
vkimage_info_hash_wvkimage_list_map_init(
    _VkImageInfoHash2WVkImageListMap * self ,
    qo_uint32_t                        capacity
);
void
vkimage_info_hash_wvkimage_list_map_destroy(
    _VkImageInfoHash2WVkImageListMap * self
);
qo_stat_t
vkimage_info_hash_wvkimage_list_map_set(
    _VkImageInfoHash2WVkImageListMap * self ,
    _VkImageInfoCompositeKey *        key ,
    _WVkImageList *                    list
);

qo_bool_t
vkimage_info_hash_wvkimage_list_map_search(
    _VkImageInfoHash2WVkImageListMap * self ,
    _VkImageInfoCompositeKey *        key ,
    _WVkImageList **                   p_list 
);
