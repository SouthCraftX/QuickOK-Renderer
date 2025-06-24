#include "depth_stencil_state.h"
#include "../funnel_hash_table.h"
#include <xxh3.h>

qo_bool_t
vk_stencil_op_state_compare(
    VkStencilOpState const * const x ,
    VkStencilOpState const * const y
) {
    return 
        x->failOp == y->failOp &&
        x->passOp == y->passOp &&
        x->depthFailOp == y->depthFailOp &&
        x->compareOp == y->compareOp &&
        x->compareMask == y->compareMask &&
        x->writeMask == y->writeMask &&
        x->reference == y->reference
    ;
}

qo_bool_t
depth_stencil_state_desc_compare(
    fht_key_t x ,
    fht_key_t y
) {
    _DepthStencilStateDesc const * const desc_x = (_DepthStencilStateDesc const *)x;
    _DepthStencilStateDesc const * const desc_y = (_DepthStencilStateDesc const *)y;
    return 
        desc_x->depth_bounds_test_enable == desc_y->depth_bounds_test_enable &&
        desc_x->depth_write_enable == desc_y->depth_write_enable &&
        desc_x->depth_compare_op == desc_y->depth_compare_op &&
        desc_x->stencil_test_enable == desc_y->stencil_test_enable &&
        desc_x->min_depth_bounds == desc_y->min_depth_bounds && // TODO: epsilon
        desc_x->max_depth_bounds == desc_y->max_depth_bounds &&
        vk_stencil_op_state_compare(&desc_x->front, &desc_y->front) &&
        vk_stencil_op_state_compare(&desc_x->back, &desc_y->back)
    ;
}
