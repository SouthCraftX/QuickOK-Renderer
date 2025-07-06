#include "rendering_env.h"
#include <vulkan/vulkan_core.h>

struct __VertexInputLayout
{
    VkVertexInputBindingDescription *   bindings; //< Where are data from
    VkVertexInputAttributeDescription * attributes; //< What are data
};
typedef struct __VertexInputLayout _VertexInputLayout ;