#include "wrapped_vulkan_image.h"
#include "mimalloc_vulkan_callback.h"
#include "rendering_env.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#include <xxh3.h>

/* Compare functions */
// We can't directly use memcmp() because padding in structure may cause false negative
qo_bool_t
vk_component_mapping_compare(
    VkComponentMapping const * lhs ,
    VkComponentMapping const * rhs
) {
    return (
        lhs->r == rhs->r &&
        lhs->g == rhs->g &&
        lhs->b == rhs->b &&
        lhs->a == rhs->a
    );
}

qo_bool_t
vk_image_subresource_range_compare(
    VkImageSubresourceRange const * lhs ,
    VkImageSubresourceRange const * rhs
) {
    return (
        lhs->aspectMask == rhs->aspectMask &&
        lhs->baseMipLevel == rhs->baseMipLevel &&
        lhs->levelCount == rhs->levelCount &&
        lhs->baseArrayLayer == rhs->baseArrayLayer &&
        lhs->layerCount == rhs->layerCount
    );
}

qo_bool_t
vk_image_view_create_info_compare(
    VkImageViewCreateInfo const * lhs ,
    VkImageViewCreateInfo const * rhs
) {
    return (
        lhs->sType == rhs->sType &&
        vk_component_mapping_compare(&lhs->components , &rhs->components) &&
        vk_image_subresource_range_compare(&lhs->subresourceRange , &rhs->subresourceRange)
    );
}

#define INIT_HASH_SEED 2166136261u

VkImageViewCreateInfo_hash_t
hash_vk_image_view_creation_info(
    VkImageViewCreateInfo const * s
) {
    XXH3_state_t * const state = XXH3_createState();
    XXH3_64bits_reset(state);
    XXH3_64bits_update(state , &s->image , sizeof(VkImage));
    XXH3_64bits_update(state , &s->viewType , sizeof(VkImageViewType));
    XXH3_64bits_update(state , &s->format , sizeof(VkFormat));
    XXH3_64bits_update(state , &s->components.r , sizeof(s->components.r));
    XXH3_64bits_update(state , &s->components.g , sizeof(s->components.g));
    XXH3_64bits_update(state , &s->components.b , sizeof(s->components.b));
    XXH3_64bits_update(state , &s->components.a , sizeof(s->components.a));
    XXH3_64bits_update(state , &s->subresourceRange.aspectMask , sizeof(s->subresourceRange.aspectMask));
    XXH3_64bits_update(state , &s->subresourceRange.baseMipLevel , sizeof(s->subresourceRange.baseMipLevel));
    XXH3_64bits_update(state , &s->subresourceRange.levelCount , sizeof(s->subresourceRange.levelCount));
    XXH3_64bits_update(state , &s->subresourceRange.baseArrayLayer , sizeof(s->subresourceRange.baseArrayLayer));
    XXH3_64bits_update(state , &s->subresourceRange.layerCount , sizeof(s->subresourceRange.layerCount));
    const XXH64_hash_t hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}

VkResult
wvkimage_new(
    _WVkImage ** p_self,
    _VkDeviceContext * device_context,
    VkImageCreateInfo const * create_info,
    VmaAllocationCreateInfo const * alloc_info,
    qo_bool_t create_default_view
) {
    _WVkImage * self = mi_malloc_tp(_WVkImage);
    //wvkimage_init()
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult ret = wvkimage_allocate_memory(self , alloc_info);
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    if (create_default_view)
    {
        ret = wvkimage_create_default_view(self);
        if (ret != VK_SUCCESS)
        {
            // TODO: other cleanup
            // mi_free(self);
            return ret;
        }
    }
    *p_self = self;
}

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
    memset(self , 0 , sizeof(_WVkImage));
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

// Cases that default view shouldn't be used:
// - Rendering to a Cubemap Face
//   -> Why: self->default_view is VK_IMAGE_VIEW_TYPE_CUBE, which represents
//           all six faces. You can't bind a view containing 6 layers to
//           VkFramebuffer that only expects a single 2D image as a attachment
//   -> Alt: In a loop, create a temporary and specific view for each face.
// - Rendering to a Specific Array Layer
//   -> Why: Same as above. The default view represents all layers, while the
//           the attachment of VkFramebuffer only expects a single 2D image.
//   -> Alt: TODO:
// - Rendering to a Specific Mip Level
//   -> Why: Same as above. The attachment of VkFramebuffer expects a 2D view
//           with a defined size. The default view contains all mip levels,
//           whose size is not unique.
//   -> Alt: TODO:
// - Format Reinterpretation
//   -> Why: The format of the default view is same with the format of the image.
//   -> Alt: I.  Create a VkImage with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
//           II. Construct VkImageViewCreateInfo manually, then set `format`
//               field to the desired and compatible format.
// So when to use it?
// - As a input texture of post-processing.
// - As a general 2D rendering target
// - As a full depth buffer
VkResult
wvkimage_create_default_view(
    _WVkImage * self
) {
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = self->image,
        .format = self->create_info.format,
        .components = {}
    };
    switch (self->create_info.imageType)
    {
        case VK_IMAGE_TYPE_1D:
            view_info.viewType = (self->create_info.arrayLayers > 1) ? 
                VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;

        case VK_IMAGE_TYPE_2D:
            if (self->create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            {
                view_info.viewType = (self->create_info.arrayLayers > 1) ? 
                    VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else {
                view_info.viewType = (self->create_info.arrayLayers > 1) ? 
                    VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            }
            break;

        case VK_IMAGE_TYPE_3D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;

        default:
            return INT32_MAX; // Unsupported image type for default view creation
    }
    
    if (self->create_info.format >= VK_FORMAT_D16_UNORM && self->create_info.format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
    { // This is a depth/stencil format.
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (self->create_info.format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else { // We assume it is a color format
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = self->mip_levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = self->array_layers;

    return vkCreateImageView(self->device_context->logical_device , &view_info , &g_vk_mimallocator , &self->default_view);
}

VkImageView
wvkimage_get_view1(
    _WVkImage * self ,
    VkImageViewCreateInfo const * view_info
) {

}

VkImageView
wvkimage_get_view2(
    _WVkImage * self ,
    VkImageSubresourceRange const * subresource_range 
) {
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = self->image,
        .format = self->format,
        .subresourceRange = *subresource_range,
    };
    if (self->array_layers > 1)
    {
        //create_info.viewType = //(create_info.imageInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    else if (self->extent.height > 1) 
    {
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else {
        create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
    }
}

void
wvkimage_destroy(
    _WVkImage * self
) {
    // TODO: Free hash table 

    if (self->default_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(self->device_context->logical_device , self->default_view , &g_vk_mimallocator);
        self->default_view = VK_NULL_HANDLE;
    }
    if (self->image != VK_NULL_HANDLE)
    {
        if (self->allocation != VK_NULL_HANDLE)
        {
            vmaDestroyImage(g_vk_global_context.vma_allocator , self->image , self->allocation);
        }
        else {
            vkDestroyImage(self->device_context->logical_device , self->image , &g_vk_mimallocator);
        }
    }
    memset(self , 0 , sizeof(_WVkImage));
}

VkResult
wvkimage_create_aliased(

) {
    
}