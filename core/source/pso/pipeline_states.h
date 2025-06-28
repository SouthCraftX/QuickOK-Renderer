#include "../rendering_env.h"
#include <vulkan/vulkan_core.h>

struct __RasterizationStateDesc
{
    VkPolygonMode   polygon_mode;
    VkCullModeFlags cull_mode;
    VkFrontFace     front_face;
    VkBool32        depth_clamp_enable;
    VkBool32        rasterizer_discard_enable;
    VkBool32        depth_bias_enable;
    qo_fp32_t       line_width;
};
typedef struct __RasterizationStateDesc _RasterizationStateDesc;

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

struct __ColorBlendAttachmentState
{
    VkPipelineColorBlendAttachmentState state;
};

struct __ColorBlendStateDesc
{

};
