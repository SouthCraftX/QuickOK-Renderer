#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPPED_VULKAN_BUFFER_SRC__

#include "../vkmemory_block.h"

struct __WVkBuffer
{
    qo_ref_count_t      reference_count;
    qo_pointer_t        mapped_data;
    VkBuffer            vkbuffer;
    VkBufferCreateInfo  buffer_info;
    _VkMemoryBlock *    memory_block;
};
typedef struct __WVkBuffer _WVkBuffer;

QO_NODISCARD
VkResult
wvkbuffer_map(
    _WVkBuffer * self
);

void
wvkbuffer_unmap(
    _WVkBuffer * self
);

VkDeviceSize
wvkbuffer_get_size(
    _WVkBuffer * self
);

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
VkBuffer
wvkbuffer_get_handle(
    _WVkBuffer * self
) {
    return self->vkbuffer;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
qo_pointer_t
wvkbuffer_get_data(
    _WVkBuffer * self
) {
    return self->mapped_data;
}


QO_GLOBAL_UNIQUE
void
wvkbuffer_get_create_info(
    _WVkBuffer *         self ,
    VkBufferCreateInfo * p_create_info
) {
    *p_create_info = self->buffer_info;
}

void
wvkbuffer_flush(
    _WVkBuffer *  self ,
    VkDeviceSize  offset ,
    VkDeviceSize  size
);

QO_FORCE_INLINE
void
wvkbuffer_full_flush(
    _WVkBuffer * self
) {
    wvkbuffer_flush(self , 0 , VK_WHOLE_SIZE);
}

QO_NODISCARD
VkResult
wvkbuffer_upload_data(
    _WVkBuffer *   self ,
    qo_cpointer_t  data ,
    VkDeviceSize   size
);

QO_NODISCARD
VkResult
wvkbuffer_new(
    _WVkBuffer **              p_self ,
    VkBufferCreateInfo const * buffer_info ,
    VmaMemoryUsage             memory_usage ,
    VmaAllocationCreateInfo    allocation_flags
);

void
wvkbuffer_unref(
    _WVkBuffer * self
);
