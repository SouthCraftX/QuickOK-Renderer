#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPPED_VULKAN_BUFFER_SRC__

#include "rendering_env.h"

struct __WVkBuffer
{
    qo_ref_count_t      reference_count;
    VkBuffer            vkbuffer;
    VmaAllocation       allocation;
    VkDeviceSize        size;
    qo_pointer_t        mapped_data;
};
typedef struct __WVkBuffer _WVkBuffer;

VkResult
wvkbuffer_map(
    _WVkBuffer *    self
);

void
wvkbuffer_unmap(
    _WVkBuffer *    self
);

void
wvkbuffer_flush(
    _WVkBuffer *    self ,
    VkDeviceSize    offset ,
    VkDeviceSize    size
);

QO_FORCE_INLINE
void
wvkbuffer_full_flush(
    _WVkBuffer *    self
) {
    wvkbuffer_flush(self, 0, VK_WHOLE_SIZE);
}

VkResult
wvkbuffer_upload_data(
    _WVkBuffer *    self ,
    qo_cpointer_t   data ,
    VkDeviceSize    size
);

VkResult
wvkbuffer_new(
    _WVkBuffer **               p_self,
    VkBufferCreateInfo const *  buffer_info ,
    VmaMemoryUsage              memory_usage,
    VmaAllocationCreateInfo     allocation_flags
);

VkResult
wvkbuffer_unref(
    _WVkBuffer *    self
);
