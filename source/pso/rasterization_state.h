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
