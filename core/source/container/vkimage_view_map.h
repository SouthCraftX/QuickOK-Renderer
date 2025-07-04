#pragma once
#define __QOR_VK_IMAGE_VIEW_MAP_SRC__

#include "../wrapped_vulkan_objects/wvkimage.h"
#include "funnel_hash_table.h"

struct __VkImageViewMap
{
    _FunnelHashTable    hash_table;
};
typedef struct __VkImageViewMap _VkImageViewMap;

qo_stat_t
vkimage_view_map_init(
    _VkImageViewMap *   self
);

void
vkimage_view_map_destroy(
    _VkImageViewMap *   self
);

qo_bool_t
vkimage_view_map_insert(
    _VkImageViewMap *               self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView                     value
);

qo_bool_t
vkimage_view_map_set(
    _VkImageViewMap *               self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView                     value
);

qo_bool_t
vkimage_view_map_search(
    _VkImageViewMap const *         self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView *                   p_value
);

XXH64_hash_t
hash_vkimage_view_create_info(
    VkImageViewCreateInfo *         view_info
);

qo_bool_t
vkimage_view_create_info_compare1(
    VkImageViewCreateInfo const * const  s1,
    VkImageViewCreateInfo const * const  s2
);