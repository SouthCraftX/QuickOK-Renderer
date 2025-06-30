#include "wvkbuffer.h"
#include "../diagnostics/vulkan_make_debug_message.h"
#include <vulkan/vulkan_core.h>

VkResult
wvkbuffer_map(
    _WVkBuffer * self
) {
    if (!self->mapped_data)
    {
        VkResult  ret = vmaMapMemory(get_vma_allocator() , self->allocation ,
            &self->mapped_data);
        if (ret != VK_SUCCESS)
        {
            return ret;
        }
    }
    return VK_SUCCESS;
}

void
wvkbuffer_unmap(
    _WVkBuffer * self
) {
    if (self->mapped_data)
    {
        vmaUnmapMemory(get_vma_allocator() ,
            vkmemory_block_get_allocation(self->memory_block));
    }
    self->mapped_data = NULL;
}

VkResult
wvkbuffer_init_brandnew(
    _WVkBuffer *                    self ,
    VkBufferCreateInfo const *      buffer_info ,
    VmaAllocationCreateInfo const * alloc_info ,
    _VkDeviceContext *              device_context
) {
    VkBuffer  vkbuffer;
    VkResult  res = vkCreateBuffer(device_context->logical_device ,
        buffer_info , get_vk_allocator() , &vkbuffer);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    _VkMemoryBlock * memory_block;
    res = vkmemory_block_new(&memory_block , device_context ,
        &device_context->memory_requirements.buffer , alloc_info);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    res = vmaBindBufferMemory(get_vma_allocator() ,
        vkmemory_block_get_allocation(memory_block) , vkbuffer);
    if (res != VK_SUCCESS)
    {
        vkmemory_block_unref(memory_block);
        vkDestroyBuffer(device_context->logical_device , vkbuffer ,
            get_vk_allocator());
        return res;
    }

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

// VkResult
// wvkbuffer_init(
//     _WVkBuffer *                self ,
//     VkBufferCreateInfo const *  buffer_info ,
//     VmaMemoryUsage              memory_usage ,
//     VmaAllocationCreateFlags    allocation_flags
// ) {
//     VmaAllocationCreateInfo allocation_info = {
//         .usage = memory_usage ,
//         .flags = allocation_flags
//     };
//     VkResult ret = vmaCreateBuffer(get_vma_allocator() , buffer_info , &allocation_info ,
// &self->vkbuffer , &self->allocation , NULL);
//     if (ret != VK_SUCCESS)
//     {
//         return ret;
//     }
//     if (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
//     {
//         return wvkbuffer_map(self);
//     }
//     return VK_SUCCESS;
// }

VkResult
wvkbuffer_init_staging(
    _WVkBuffer *       self ,
    _VkDeviceContext * device_context ,
    VkDeviceSize       size
) {
    VkBufferCreateInfo  buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ,
        .size  = size ,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT ,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    return wvkbuffer_init(self , &buffer_info , VMA_MEMORY_USAGE_CPU_ONLY ,
        VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

VkResult
wvkbuffer_init_gpu_only(
    _WVkBuffer *        self ,
    VkDeviceSize        size ,
    VkBufferUsageFlags  usage
) {
    VkBufferCreateInfo  buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ,
        .size  = size ,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    return wvkbuffer_init(self , &buffer_info , VMA_MEMORY_USAGE_GPU_ONLY , 0);
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
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, 
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
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, 
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
