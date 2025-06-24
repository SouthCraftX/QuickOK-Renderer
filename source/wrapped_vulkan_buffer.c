#include "wrapped_vulkan_buffer.h"
#include "rendering_env.h"
#include <vulkan/vulkan_core.h>

VkResult
wvkbuffer_map(
    _WVkBuffer *    self
) {
    if (!self->mapped_data)
    {
        VkResult ret = vmaMapMemory(get_vma_allocator() , self->allocation , &self->mapped_data);
        if (ret != VK_SUCCESS)
        {
            return ret;
        }
    }
    return VK_SUCCESS;
}

void
wvkbuffer_unmap(
    _WVkBuffer *    self 
) {
    if (self->mapped_data)
    {
        vmaUnmapMemory(get_vma_allocator(), self->allocation);
    }
}

VkResult
wvkbuffer_init(
    _WVkBuffer *                self ,
    VkBufferCreateInfo const *  buffer_info ,
    VmaMemoryUsage              memory_usage ,
    VmaAllocationCreateFlags    allocation_flags
) {
    VmaAllocationCreateInfo allocation_info = {
        .usage = memory_usage ,
        .flags = allocation_flags
    };
    VkResult ret = vmaCreateBuffer(get_vma_allocator() , buffer_info , &allocation_info , &self->vkbuffer , &self->allocation , NULL);
    if (ret != VK_SUCCESS)
    {
        return ret;
    }
    if (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
    {
        return wvkbuffer_map(self);
    }
    return VK_SUCCESS;
}

VkResult
wvkbuffer_init_staging(
    _WVkBuffer *        self ,
    _VkDeviceContext *  device_context ,
    VkDeviceSize        size
) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    return wvkbuffer_init(self , &buffer_info , VMA_MEMORY_USAGE_CPU_ONLY , VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

VkResult
wvkbuffer_init_gpu_only(
    _WVkBuffer *        self,
    VkDeviceSize        size,
    VkBufferUsageFlags  usage
) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    return wvkbuffer_init(self , &buffer_info , VMA_MEMORY_USAGE_GPU_ONLY , 0);
}

VkResult
wvkbuffer_upload_data(
    _WVkBuffer *    self, 
    qo_cpointer_t    data, 
    VkDeviceSize    size
) {
    if (size > self->size)
    {
        // TODO: find a suitable return code
    }
    VkResult ret = wvkbuffer_map(self);
    if (ret != VK_SUCCESS)
    {
        return ret;
    }
    memcpy(self->mapped_data, data, size);
    wvkbuffer_unmap(self);
    return VK_SUCCESS;
}

void
wvkbuffer_flush(
    _WVkBuffer *    self,
    VkDeviceSize    offset,
    VkDeviceSize    size
) {
    vmaFlushAllocation(get_vma_allocator(), self->allocation, offset, size);
}
