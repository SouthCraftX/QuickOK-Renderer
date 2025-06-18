#include "warped_vulkan_image.h"
#include "mimalloc_vulkan_callback.h"
#include "rendering_env.h"
#include <vulkan/vulkan_core.h>

VkResult
wvkimage_simple_record_transition(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer ,
    VkImageLayout    new_layout
) {

}

VkResult
wvkimage_init(
    _WVkImage * self,
    VkImageCreateInfo const * create_info ,
    VkExtent3D  extent ,
    VkFormat    format ,
    qo_uint32_t mip_levels ,
    qo_uint32_t array_layers ,
    VkDevice    device 
) {
    return vkCreateImage(device , create_info , &g_vk_mimallocator , &self->image);
}

VkResult
wvkimage_allocate_memory(
    _WVkImage * self ,
    VmaAllocationCreateInfo const * alloc_info 
) {
    VkResult ret = vmaAllocateMemoryForImage(
        g_vk_global_context.vma_allocator ,
        self->image , 
        alloc_info , 
        &self->allocation ,
        NULL // TODO: check this argument's function/feature
    );
    if (ret != VK_SUCCESS) {
        return ret;
    }
    return vmaBindImageMemory(
        g_vk_global_context.vma_allocator ,
        self->allocation ,
        self->image 
    );
}

VkResult
wvkimage_create_aliased(

) {
    
}