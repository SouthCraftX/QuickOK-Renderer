#pragma once
#define __QOR_VKIMAGE_VIEW_HASH_WVKIMAGE_LIST_MAP_SRC__

//#include "vk_image_view_map.h"
#include "vkimage_info_hash_wvkimage_list_map.h"
#include "funnel_hash_table.h"
#include "wrapped_vulkan_image.h"

#include <xxh3.h>

qo_bool_t
vkimage_info_compare(
    fht_key_t x ,
    fht_key_t y

) {
    VkImageCreateInfo * p_x = (VkImageCreateInfo *)x;
    VkImageCreateInfo * p_y = (VkImageCreateInfo *)y;
    return p_x->imageType == p_y->imageType &&
           p_x->format == p_y->format &&
           p_x->extent.width == p_y->extent.width &&
           p_x->extent.height == p_y->extent.height &&
           p_x->extent.depth == p_y->extent.depth &&
           p_x->mipLevels == p_y->mipLevels &&
           p_x->arrayLayers == p_y->arrayLayers &&
           p_x->samples == p_y->samples &&
           p_x->tiling == p_y->tiling &&    
           p_x->usage == p_y->usage &&
           p_x->sharingMode == p_y->sharingMode &&
           p_x->initialLayout == p_y->initialLayout
    ;
}

XXH64_hash_t
vkimage_info_composite_key_hash(
    fht_key_t key ,
    qo_uint32_t salt
) {
    _VkImageInfoCompositeKey * p_key = (_VkImageInfoCompositeKey *)key;
    return p_key->precomputed_hash ^ (qo_uint64_t)salt;
}

qo_stat_t
vkimage_info_hash_wvkimage_list_map_init(
    _VkImageInfoHash2WVkImageListMap *  self , 
    qo_uint32_t                         capacity
) {
    return fht_init(
        &self->hash_table ,
        capacity ,
        .1f ,
        &(_FunnelHashTableAuxiliary){
            .hash_func = vkimage_info_composite_key_hash,
            .equals_func = vkimage_info_compare,
            .key_destroy_func = NULL
        }
    );
}

void
vkimage_info_hash_wvkimage_list_map_destroy(
    _VkImageInfoHash2WVkImageListMap *  self 
) {
    fht_destroy(&self->hash_table);
}

qo_bool_t
vkimage_info_hash_wvkimage_list_map_search(
    _VkImageInfoHash2WVkImageListMap *  self, 
    _VkImageInfoCompositeKey *             key ,
    _WVkImageList **                   p_list 
) {
    _FunnelHashTable * p_hash_table = &self->hash_table;
    return fht_search(
        p_hash_table ,
        (fht_key_t)key ,
        (fht_value_t *)p_list
    );
}

qo_stat_t
vkimage_info_hash_wvkimage_list_map_set(
    _VkImageInfoHash2WVkImageListMap *  self, 
    _VkImageInfoCompositeKey *          key ,
    _WVkImageList *                    list
) { 
    return fht_set(
        &self->hash_table ,
        (fht_key_t)&key ,
        (fht_value_t)list
    );
}

