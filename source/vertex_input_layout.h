#include "rendering_env.h"
#include <vulkan/vulkan_core.h>

struct __VertexInputLayout
{
    VkVertexInputAttributeDescription * attributes;
    VkVertexInputBindingDescription *   bindings;
};