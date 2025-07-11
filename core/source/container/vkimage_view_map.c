#include "vkimage_view_map.h"
#include "funnel_hash_table.h"
#include <vulkan/vulkan_core.h>
#include <xxh3.h>

typedef XXH64_hash_t VkImageViewCreateInfo_hash_t;

// We can't directly use memcmp() because padding in structure may cause false negative
qo_bool_t
VkComponentMapping_compare(
    VkComponentMapping const * lhs ,
    VkComponentMapping const * rhs
) {
    return
        lhs->r == rhs->r && lhs->g == rhs->g && lhs->b == rhs->b &&
        lhs->a == rhs->a
    ;
}

qo_bool_t
vkimage_subresource_range_compare(
    VkImageSubresourceRange const * lhs ,
    VkImageSubresourceRange const * rhs
) {
    return
        lhs->aspectMask == rhs->aspectMask &&
        lhs->baseMipLevel == rhs->baseMipLevel &&
        lhs->levelCount == rhs->levelCount &&
        lhs->baseArrayLayer == rhs->baseArrayLayer &&
        lhs->layerCount == rhs->layerCount
    ;
}


qo_bool_t
vkimage_view_create_info_compare1(
    VkImageViewCreateInfo const * const  s1,
    VkImageViewCreateInfo const * const  s2
) {
    return
        s1->sType == s2->sType &&
        VkComponentMapping_compare(&s1->components ,
        &s2->components) &&
        vkimage_subresource_range_compare(&s1->subresourceRange ,
        &s2->subresourceRange)
    ;
}

qo_bool_t
vkimage_view_create_info_compare2(
    fht_key_t   key1,
    fht_key_t   key2
) {
    VkImageViewCreateInfo const * const  s1 = (VkImageViewCreateInfo const *)key1;
    VkImageViewCreateInfo const * const  s2 = (VkImageViewCreateInfo const *)key2;
    return vkimage_view_create_info_compare1(s1 , s2);
}


#define INIT_HASH_SEED  2166136261u

VkImageViewCreateInfo_hash_t
hash_vkimage_view_creation_info2(
    fht_key_t                       key,
    qo_uint32_t                     salt
) {
    VkImageViewCreateInfo const * const  s = (VkImageViewCreateInfo const *)key;
    XXH3_state_t * const  state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);
    XXH3_64bits_update(state , &s->image , sizeof(VkImage));
    XXH3_64bits_update(state , &s->viewType , sizeof(VkImageViewType));
    XXH3_64bits_update(state , &s->format , sizeof(VkFormat));
    XXH3_64bits_update(state , &s->components.r , sizeof(s->components.r));
    XXH3_64bits_update(state , &s->components.g , sizeof(s->components.g));
    XXH3_64bits_update(state , &s->components.b , sizeof(s->components.b));
    XXH3_64bits_update(state , &s->components.a , sizeof(s->components.a));
    XXH3_64bits_update(state , &s->subresourceRange.aspectMask ,
        sizeof(s->subresourceRange.aspectMask));
    XXH3_64bits_update(state , &s->subresourceRange.baseMipLevel ,
        sizeof(s->subresourceRange.baseMipLevel));
    XXH3_64bits_update(state , &s->subresourceRange.levelCount ,
        sizeof(s->subresourceRange.levelCount));
    XXH3_64bits_update(state , &s->subresourceRange.baseArrayLayer ,
        sizeof(s->subresourceRange.baseArrayLayer));
    XXH3_64bits_update(state , &s->subresourceRange.layerCount ,
        sizeof(s->subresourceRange.layerCount));
    const XXH64_hash_t  hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}

void
destroy_vkimage_view_info_key(
    fht_key_t  key
) {
    mi_free((qo_pointer_t)key);
}

qo_stat_t
vkimage_view_map_init(
    _VkImageViewMap *   self
) {
    return fht_init(&self->hash_table ,
        1 ,
        .1 ,
        &(_FunnelHashTableAuxiliary){
            .hash_func = hash_vkimage_view_creation_info2,
            .equals_func = vkimage_view_create_info_compare2,
            .key_destroy_func = destroy_vkimage_view_info_key,
        });
}

