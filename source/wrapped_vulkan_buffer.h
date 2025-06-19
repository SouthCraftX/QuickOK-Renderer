#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPPED_VULKAN_BUFFER_SRC__

#include "rendering_env.h"

struct __WVkBuffer
{
    VkBuffer       vkbuffer;
    VmaAllocation  allocation;
    VkDeviceSize   size;
    qo_pointer_t   mapped_data;
};
typedef struct __WVkBuffer _WVkBuffer;
