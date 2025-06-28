#include "rasterization_state.h"
#include "../funnel_hash_table.h"
#include <xxh3.h>

qo_bool_t
rasteriaztion_state_desc_compare(
    fht_key_t x ,
    fht_key_t y
) {
    _RasterizationStateDesc const * const desc1 = (_RasterizationStateDesc const *)x;
    _RasterizationStateDesc const * const desc2 = (_RasterizationStateDesc const *)y;
    return
        desc1->polygon_mode == desc2->polygon_mode &&
        desc1->cull_mode == desc2->cull_mode &&
        desc1->front_face == desc2->front_face &&
        desc1->depth_clamp_enable == desc2->depth_clamp_enable &&
        desc1->rasterizer_discard_enable == desc2->rasterizer_discard_enable &&
        desc1->depth_bias_enable == desc2->depth_bias_enable &&
        desc1->line_width == desc2->line_width
    ;
}

XXH64_hash_t
rasterization_state_desc_hash(
    fht_key_t   key ,
    qo_int32_t  salt
) {
    _RasterizationStateDesc const * const desc = (_RasterizationStateDesc const *)key;
    XXH3_state_t * const state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);

    XXH3_64bits_update(state , &desc->polygon_mode , sizeof(VkPolygonMode));
    XXH3_64bits_update(state , &desc->cull_mode , sizeof(VkCullModeFlags));
    XXH3_64bits_update(state , &desc->front_face , sizeof(VkFrontFace));
    XXH3_64bits_update(state , &desc->depth_clamp_enable , sizeof(VkBool32));
    XXH3_64bits_update(state , &desc->rasterizer_discard_enable , sizeof(VkBool32));
    XXH3_64bits_update(state , &desc->depth_bias_enable , sizeof(VkBool32));
    XXH3_64bits_update(state , &desc->line_width , sizeof(qo_fp32_t));

    const XXH64_hash_t hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}
