#pragma once
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
    _VkMemoryBlock **               p_self ,
    _VkDeviceContext *              device_context ,
    VkMemoryRequirements const *    memory_requirements ,
    VmaAllocationCreateInfo const * alloc_info
) {
    _VkMemoryBlock * self = mi_malloc_tp(_VkMemoryBlock);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkResult  res = vmaAllocateMemory(get_vma_allocator() ,
        memory_requirements , alloc_info , &self->allocation ,
        &self->allocation_info);
    if (res != VK_SUCCESS)
    {
        mi_free(self);
        return res;
    }

    self->reference_count = 1;
    self->device_context = device_context;
    *p_self = self;
    return VK_SUCCESS;
}

QO_GLOBAL_UNIQUE
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
vkmemory_block_unref(
    _VkMemoryBlock *    self
) {
    if (self)
    {
        self->reference_count--;
        if (self->reference_count)
            return;
        vmaFreeMemory(get_vma_allocator() , self->allocation);
        mi_free(self);
    }
}