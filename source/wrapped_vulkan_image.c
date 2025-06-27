#include "wrapped_vulkan_image.h"
#include "mimalloc_vulkan_callback.h"
#include "access_info_inferor.h"
#include "rendering_env.h"
#include "vkimage_view_map.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#include <xxh3.h>

/* Compare functions */



VkResult
wvkimage_new(
    _WVkImage **                    p_self ,
    _VkDeviceContext *              device_context ,
    VkImageCreateInfo const *       create_info ,
    VmaAllocationCreateInfo const * alloc_info ,
    qo_bool_t                       create_default_view
) {
    _WVkImage * self = mi_malloc_tp(_WVkImage);
    //wvkimage_init()
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkimage_allocate_memory(self , alloc_info);
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
            // not simply mi_free(self);
            return ret;
        }
    }
    self->device_context = device_context;
    *p_self = self;
}

void
wvkimage_record_layout_transition1(
    _WVkImage *                  self ,
    VkCommandBuffer              command_buffer ,
    VkImageMemoryBarrier const * barrier ,
    VkPipelineStageFlags         source_stage_mask ,
    VkPipelineStageFlags         target_stage_mask
) {
    if (!source_stage_mask)
    {
        source_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (!target_stage_mask)
    {
        target_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    vkCmdPipelineBarrier(command_buffer , source_stage_mask ,
        target_stage_mask , 0 , 0 , NULL , 0 , NULL , 1 , barrier);
    self->current_layout = barrier->newLayout;
}

void
wvkimage_record_layout_transition2(
    _WVkImage *                     self ,
    VkCommandBuffer                 command_buffer ,
    VkImageLayout                   new_layout ,
    qo_uint32_t                     source_queue_family ,
    qo_uint32_t                     target_queue_family ,
    VkImageSubresourceRange const * p_subresource_range
) {
    VkImageLayout  old_layout = self->current_layout;
    if (old_layout == new_layout &&
        source_queue_family == target_queue_family &&
        p_subresource_range == NULL)
    {
        return;
    }

    _LegacyBarrierAccessInfo  source_info =
        infer_legacy_access_info(old_layout);
    _LegacyBarrierAccessInfo  target_info =
        infer_legacy_access_info(new_layout);

    VkImageMemoryBarrier  barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER ,
        .srcAccessMask = source_info.access_mask ,
        .dstAccessMask = target_info.access_mask ,
        .oldLayout = self->current_layout ,
        .newLayout = new_layout ,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED ,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED ,
        .image = self->image
    };

    if (p_subresource_range)
    {
        barrier.subresourceRange = *p_subresource_range;
    }
    else {
        barrier.subresourceRange = (VkImageSubresourceRange) {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ,
            .baseMipLevel = 0 ,
            .levelCount = self->mip_levels ,
            .baseArrayLayer = 0 ,
            .layerCount = self->array_layers
        };
    }
    wvkimage_record_layout_transition1(self , command_buffer , &barrier ,
        source_info.stage_mask , target_info.stage_mask);
}

void
wvkimage_simple_record_transition(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer ,
    VkImageLayout    new_layout
) {
    wvkimage_record_layout_transition2(self , command_buffer , new_layout ,
        VK_QUEUE_FAMILY_IGNORED , VK_QUEUE_FAMILY_IGNORED , NULL);
}

// TODO: Move it to header
VkDescriptorImageInfo
wvkimage_make_descriptor_info(
    _WVkImage * self ,
    VkSampler   sampler
) {
    VkDescriptorImageInfo  descriptor_info = {
        .imageLayout = self->current_layout ,
        .imageView = self->default_view ,
        .sampler = sampler
    };
    return descriptor_info;
}

VkResult
wvkimage_init(
    _WVkImage *               self ,
    VkImageCreateInfo const * create_info ,
    VkExtent3D                extent ,
    VkFormat                  format ,
    qo_uint32_t               mip_levels ,
    qo_uint32_t               array_layers ,
    _VkDeviceContext *       device_context
) {
    memset(self , 0 , sizeof(_WVkImage));
    return vkCreateImage(device_context->logical_device , create_info , get_vk_allocator() ,
        &self->image);
}

VkResult
wvkimage_allocate_memory(
    _WVkImage *                     self ,
    VmaAllocationCreateInfo const * alloc_info
) {
    VkResult  ret = vmaAllocateMemoryForImage(
        g_vk_global_context.vma_allocator , self->image , alloc_info ,
        &self->allocation , NULL // TODO: check this argument's function/feature
    );
    if (ret != VK_SUCCESS)
    {
        return ret;
    }
    return vmaBindImageMemory(
        g_vk_global_context.vma_allocator , self->allocation , self->image
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
    VkImageViewCreateInfo  view_info = {
        .sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
        .image  = self->image ,
        .format = self->create_info.format ,
        .components = {}
    };
    switch (self->create_info.imageType)
    {
        case VK_IMAGE_TYPE_1D:
            view_info.viewType = (self->create_info.arrayLayers > 1) ?
                                 VK_IMAGE_VIEW_TYPE_1D_ARRAY :
                                 VK_IMAGE_VIEW_TYPE_1D;
            break;

        case VK_IMAGE_TYPE_2D:
            if (self->create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
            {
                view_info.viewType = (self->create_info.arrayLayers > 1) ?
                                     VK_IMAGE_VIEW_TYPE_CUBE_ARRAY :
                                     VK_IMAGE_VIEW_TYPE_CUBE;
            }
            else {
                view_info.viewType = (self->create_info.arrayLayers > 1) ?
                                     VK_IMAGE_VIEW_TYPE_2D_ARRAY :
                                     VK_IMAGE_VIEW_TYPE_2D;
            }
            break;

        case VK_IMAGE_TYPE_3D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;

        default:
            return INT32_MAX; // Unsupported image type for default view creation
    }

    if (self->create_info.format >= VK_FORMAT_D16_UNORM &&
        self->create_info.format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
    { // This is a depth/stencil format.
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (self->create_info.format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            view_info.subresourceRange.aspectMask |=
                VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else { // We assume it is a color format
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = self->mip_levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = self->array_layers;

    return vkCreateImageView(self->device_context->logical_device , &view_info ,
        get_vk_allocator() , &self->default_view);
}

VkResult
wvkimage_get_view1(
    _WVkImage *                   self ,
    VkImageViewCreateInfo const * view_info ,
    VkImageView *                 p_view
) {
    QO_ASSERT(view_info->image == self->image);

    VkImageView found_view;
    if (vk_image_view_map_search(&self->view_map, view_info, &found_view))
    { // Cache hit
        *p_view = found_view;
        return VK_SUCCESS;
    }

    VkImageView new_view;
    VkResult ret = vkCreateImageView(self->device_context->logical_device , view_info , get_vk_allocator() , &new_view);
    if (ret != VK_SUCCESS)
    {
        return ret;
    }
    vk_image_view_map_insert(&self->view_map, view_info, new_view);
    *p_view = new_view;
    return VK_SUCCESS;
}

VkResult
wvkimage_get_view2(
    _WVkImage *                     self ,
    VkImageSubresourceRange const * subresource_range ,
    VkImageViewType                 auto_or_user_type ,
    VkImageView *                   p_view
) {
    VkImageViewCreateInfo  create_info = {
        .sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
        .image  = self->image ,
        .format = self->format ,
        .subresourceRange = *subresource_range ,
        .components = {} 
    };

    if (auto_or_user_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM)
    {
        // TODO: Same as default view creation. Extract it out!
    }
    else 
    {
        create_info.viewType = auto_or_user_type;
    }

    return wvkimage_get_view1(self, &create_info, p_view);

    // if (self->array_layers > 1)
    // {
    //     //create_info.viewType = //(create_info.imageInfo.flags &
    //     // VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? VK_IMAGE_VIEW_TYPE_CUBE :
    //     // VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    // }
    // else if (self->extent.height > 1)
    // {
    //     create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    // }
    // else {
    //     create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
    // }
}

VkResult
wvkimage_generate_mipmaps(
    _WVkImage * self ,
    VkCommandBuffer command_buffer
) {
    if (self->mip_levels <= 1)
    {
        // TODO: work out a return code
    }
    

}

void
wvkimage_destroy(
    _WVkImage * self
) {
    // TODO: Free hash table

    if (self->default_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(self->device_context->logical_device ,
            self->default_view , get_vk_allocator());
        self->default_view = VK_NULL_HANDLE;
    }
    if (self->image != VK_NULL_HANDLE)
    {
        if (self->allocation != VK_NULL_HANDLE)
        {
            vmaDestroyImage(g_vk_global_context.vma_allocator , self->image ,
                self->allocation);
        }
        else {
            vkDestroyImage(self->device_context->logical_device , self->image ,
                get_vk_allocator());
        }
    }
    memset(self , 0 , sizeof(_WVkImage));
}

VkResult
wvkimage_create_aliased(

) {
}
