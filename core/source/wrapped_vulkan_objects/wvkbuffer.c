#include "wvkbuffer.h"
#include "../diagnostics/vulkan_make_debug_message.h"
#include <mimalloc.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

VkResult
wvkbuffer_init_brandnew(
    _WVkBuffer *                    self ,
    VkBufferCreateInfo const *      buffer_info ,
    VmaAllocationCreateInfo const * alloc_create_info ,
    _VkDeviceContext *              device_context
) {
    VkBuffer  vkbuffer;
    VmaAllocation  allocation;
    VmaAllocationInfo  alloc_info;
    VkResult  res = vmaCreateBuffer(get_vma_allocator() , buffer_info ,
        &alloc_create_info , &vkbuffer , &allocation , &alloc_info);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    _VkMemoryBlock * memory_block;
    res = vkmemory_block_wrap(&memory_block , device_context , allocation ,
        &alloc_info);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    self->mapped_data = vkmemory_block_get_mapped_data(memory_block);
    self->vkbuffer = vkbuffer;
    self->buffer_info  = *buffer_info;
    self->memory_block = memory_block;
    self->mapped_data  = NULL;
    return VK_SUCCESS;
}

// VkResult

VkResult
wvkbuffer_init_aliased(
    _WVkBuffer *               self ,
    VkBufferCreateInfo const * buffer_info ,
    _VkMemoryBlock *           memory_block ,
    VkDeviceSize               offset
) {
    VkBuffer  vkbuffer;
    VkResult  res =
        vkCreateBuffer(memory_block->device_context->logical_device ,
            buffer_info , get_vk_allocator() , &vkbuffer);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    VmaAllocation  source_allocation =
        vkmemory_block_get_allocation(memory_block);
    VmaAllocationInfo  source_alloc_info;
    vmaGetAllocationInfo(get_vma_allocator() , source_allocation ,
        &source_alloc_info);

    res = vkBindBufferMemory(memory_block->device_context->logical_device ,
        vkbuffer , source_alloc_info.deviceMemory ,
        source_alloc_info.offset + offset);
    if (res != VK_SUCCESS)
    {
        vkDestroyBuffer(memory_block->device_context->logical_device ,
            vkbuffer , get_vk_allocator());
        return res;
    }
    self->vkbuffer = vkbuffer;
    self->buffer_info  = *buffer_info;
    self->memory_block = memory_block;
    self->mapped_data  = NULL;
    return VK_SUCCESS;
}

VkResult
wvkbuffer_new(
    _WVkBuffer **                   p_self ,
    VkBufferCreateInfo const *      buffer_info ,
    VmaAllocationCreateInfo const * alloc_info ,
    _VkDeviceContext *              device_context
) {
    _WVkBuffer * self = mi_malloc_tp(_WVkBuffer);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkbuffer_init_brandnew(self , buffer_info , alloc_info ,
        device_context);
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    *p_self = self;
    return VK_SUCCESS;
}

VkResult
wvkbuffer_new_aliased(
    _WVkBuffer **              p_self ,
    VkBufferCreateInfo const * buffer_info ,
    _VkMemoryBlock *           memory_block ,
    VkDeviceSize               offset
) {
    _WVkBuffer * self = mi_malloc_tp(_WVkBuffer);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkbuffer_init_aliased(self , buffer_info , memory_block ,
        offset);
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    *p_self = self;
    return VK_SUCCESS;
}

VkResult
wvkbuffer_new_cpu_only(
    _WVkBuffer **       p_self ,
    _VkDeviceContext *  device_context ,
    VkDeviceSize        size ,
    VkBufferUsageFlags  usage
) {
    VkBufferCreateInfo  buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ,
        .pNext = NULL ,
        .flags = 0 ,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE ,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
        .size  = size ,
        .queueFamilyIndexCount = 0 ,
        .pQueueFamilyIndices = NULL
    };
    VmaAllocationCreateInfo  alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY ,
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT ,
        .requiredFlags  = 0 ,
        .preferredFlags = 0 ,
        .memoryTypeBits = 0 ,
        .pool = VK_NULL_HANDLE ,
        .pUserData = NULL ,
        .priority  = 0.
    };
    return wvkbuffer_new(p_self , &buffer_info , &alloc_info , device_context);
}

VkResult
wvkbuffer_new_gpu_only(
    _WVkBuffer **       p_self ,
    _VkDeviceContext *  device_context ,
    VkDeviceSize        size ,
    VkBufferUsageFlags  usage ,
    qo_uint32_t *       queue_family_indices , // nullable
    qo_uint32_t         queue_family_indices_count
) {
    VkBufferCreateInfo  buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ,
        .pNext = NULL ,
        .flags = 0 ,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
        .size  = size
    };

    if (queue_family_indices_count > 1)
    {
        buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        buffer_info.queueFamilyIndexCount = queue_family_indices_count;
        buffer_info.pQueueFamilyIndices = queue_family_indices;
    }
    else {
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.queueFamilyIndexCount = 0;
        buffer_info.pQueueFamilyIndices = NULL;
    }

    VmaAllocationCreateInfo  alloc_info = {
        .flags = 0 ,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY ,
        .requiredFlags  = 0 ,
        .preferredFlags = 0 ,
        .memoryTypeBits = 0 ,
        .pool = VK_NULL_HANDLE ,
        .pUserData = NULL ,
        .priority  = 0.
    };
    return wvkbuffer_new(p_self , &buffer_info , &alloc_info , device_context);
}

VkResult
wvkbuffer_upload_data(
    _WVkBuffer *   self ,
    qo_cpointer_t  data ,
    VkDeviceSize   size ,
    VkDeviceSize   offset
) {
    VmaAllocationInfo  alloc_info;
    vmaGetAllocationInfo(get_vma_allocator() ,
        vkmemory_block_get_allocation(self->memory_block) , &alloc_info);

    if (alloc_info.memoryType & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        if (self->mapped_data)
        {
            memcpy((qo_pointer_t) self->mapped_data + offset , data , size);
            return VK_SUCCESS;
        }
        vulkan_send_debug_message(
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ,
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ,
            "Call contract violation: memory must be mapped before uploading data to GPU."
        );
        return VK_ERROR_MEMORY_MAP_FAILED;
    }
    else if (self->buffer_info.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    {
    }
    else
    {
        vulkan_send_debug_message(
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ,
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ,
            "wvkbuffer_upload_data() called on a GPU-Only buffer that doesn't have the TRANSFER_DST usage flag"
        );
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }
}

void
wvkbuffer_flush(
    _WVkBuffer *  self ,
    VkDeviceSize  offset ,
    VkDeviceSize  size
) {
    vmaFlushAllocation(get_vma_allocator() ,
        vkmemory_block_get_allocation(self->memory_block) , offset , size);
}

void
wvkbuffer_destory(
    _WVkBuffer * self
) {
    // unmap not needed, because consistent mapped memory will be automatically
    // handled when VmaAllocation is destroyed.
    vkDestroyBuffer(self->memory_block->device_context->logical_device ,
        self->vkbuffer , get_vk_allocator());
    vkmemory_block_unref(self->memory_block);
}

void
wvkbuffer_unref(
    _WVkBuffer * self
) {
    self->reference_count--;
    if (self->reference_count)
    {
        return;
    }
    wvkbuffer_destory(self);
    mi_free(self);
}
