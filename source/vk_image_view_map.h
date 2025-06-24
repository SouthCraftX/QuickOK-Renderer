#pragma once
#define __QOR_VK_IMAGE_VIEW_MAP_SRC__

#include "funnel_hash_table.h"

struct __VkImageViewMap
{
    _FunnelHashTable    hash_table;
};
typedef struct __VkImageViewMap _VkImageViewMap;

qo_stat_t
vk_image_view_map_init(
    _VkImageViewMap *   self
);

void
vk_image_view_map_destroy(
    _VkImageViewMap *   self
);

qo_bool_t
vk_image_view_map_insert(
    _VkImageViewMap *               self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView                     value
);

qo_bool_t
vk_image_view_map_set(
    _VkImageViewMap *               self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView                     value
);

qo_bool_t
vk_image_view_map_search(
    _VkImageViewMap const *         self ,
    VkImageViewCreateInfo const *   key ,
    VkImageView *                   p_value
);


