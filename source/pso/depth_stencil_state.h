#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_PSO_DEPTH_STENCIL_STATE_SRC__

#include "../rendering_env.h"

struct __DepthStencilStateDesc
{
    // --- Depth Test ---
    VkBool32            depth_test_enable;
    VkBool32            depth_write_enable;
    VkCompareOp         depth_compare_op;
    VkBool32            depth_bounds_test_enable;
    qo_fp32_t           min_depth_bounds;
    qo_fp32_t           max_depth_bounds;

    // --- Stencil Test ---
    VkBool32            stencil_test_enable;
    VkStencilOpState    front;
    VkStencilOpState    back;
};
typedef struct __DepthStencilStateDesc _DepthStencilStateDesc;