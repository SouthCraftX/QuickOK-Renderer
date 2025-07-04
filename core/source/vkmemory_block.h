#pragma once
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#define __QOR_VK_MEMORY_BLOCK_SRC__

#include "../rendering_env.h"

struct __VkMemoryBlock
{
    qo_ref_count_t     reference_count;
    _VkDeviceContext * device_context;
    VmaAllocation      allocation;
    VmaAllocationInfo  allocation_info;  // very useful in debug
};
typedef struct __VkMemoryBlock _VkMemoryBlock;

QO_GLOBAL_UNIQUE
VkResult
vkmemory_block_new(
    _VkMemoryBlock **            p_self ,
    _VkDeviceContext *           device_context ,
    VkMemoryRequirements const * memory_requirements ,
    VmaAllocationCreateInfo      alloc_info
) {
    _VkMemoryBlock * self = mi_malloc_tp(_VkMemoryBlock);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    alloc_info.memoryTypeBits = memory_requirements->memoryTypeBits;
    VkResult  res = vmaAllocateMemory(get_vma_allocator() ,
        memory_requirements , &alloc_info , &self->allocation ,
        &self->allocation_info);
    if (res != VK_SUCCESS)
    {
        mi_free(self);
        return res;
    }

    // self->reference_count = 1;
    // self->device_context = device_context;
    // *p_self = self;
    // return VK_SUCCESS;
}

QO_NODISCARD
VkResult
vkmemory_block_wrap(
    _VkMemoryBlock **         p_self ,
    _VkDeviceContext *        device_context ,
    VmaAllocation             allocation ,
    VmaAllocationInfo const * alloc_info
) {
    _VkMemoryBlock * self = mi_malloc_tp(_VkMemoryBlock);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    self->reference_count = 1;
    self->device_context  = device_context;
    self->allocation = allocation;
    self->allocation_info = *alloc_info;
    return VK_SUCCESS;
}

QO_GLOBAL_UNIQUE QO_NODISCARD
VmaAllocation
vkmemory_block_get_allocation(
    _VkMemoryBlock * self
) {
    return self->allocation;
}

QO_GLOBAL_UNIQUE
qo_ref_count_t
vkmemory_block_ref(
    _VkMemoryBlock * self
) {
    self->reference_count++;
    return self->reference_count;
}

QO_GLOBAL_UNIQUE
void
vkmemory_block_flush(
    _VkMemoryBlock *    self ,
    VkDeviceSize        offset ,
    VkDeviceSize        size
) {
    vmaFlushAllocation(get_vma_allocator() , self->allocation , offset , size);
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
qo_pointer_t
vkmemory_block_get_mapped_data(
    _VkMemoryBlock * self
) {
    return self->allocation_info.pMappedData;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
VkDeviceMemory
vkmemory_block_get_device_memory(
    _VkMemoryBlock *    self
) {
    return self->allocation_info.deviceMemory;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
VkDeviceSize
vkmemory_block_get_size(
    _VkMemoryBlock *    self
) {
    return self->allocation_info.size;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
VkDeviceSize
vkmemory_block_get_offset(
    _VkMemoryBlock *    self
) {
    return self->allocation_info.offset;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
VmaAllocationInfo const *
vkmemory_block_get_allocation_info(
    _VkMemoryBlock *    self
) {
    return &self->allocation_info;
}

QO_GLOBAL_UNIQUE
void
vkmemory_block_unref(
    _VkMemoryBlock * self
) {
    if (self)
    {
        self->reference_count--;
        if (self->reference_count)
        {
            return;
        }
        vmaFreeMemory(get_vma_allocator() , self->allocation);
        mi_free(self);
    }
}
