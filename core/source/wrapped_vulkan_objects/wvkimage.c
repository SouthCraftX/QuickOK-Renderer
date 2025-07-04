#include "wvkimage.h"
#include "../access_info_inferor.h"
#include "../rendering_env.h"
#include "wvkimage_memory.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#include <xxh3.h>

VkResult
wvkimage_init_brandnew(
    _WVkImage *                     self ,
    VkImageCreateInfo const *       create_info ,
    VkExtent3D                      extent ,
    VkFormat                        format ,
    qo_uint32_t                     mip_levels ,
    qo_uint32_t                     array_layers ,
    _VkDeviceContext *              device_context ,
    VmaAllocationCreateInfo const * alloc_info
) {
    VkResult  res = vkCreateImage(self->device_context->logical_device ,
        create_info , get_vk_allocator() , &self->image);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    _VkMemoryBlock * memory_block;
    res = vkmemory_block_new(&memory_block , device_context ,
        &device_context->memory_requirements.image , *alloc_info);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    self->extent = extent;
    self->format = format;
    self->mip_levels = mip_levels;
    self->array_layers = array_layers;
    self->current_layout = create_info->initialLayout;
    self->device_context = device_context;
}

VkResult
wvkimage_bind_memory(
    _WVkImage *      self ,
    _VkMemoryBlock * memory_block ,
    VkDeviceSize     offset
) {
    return vkBindImageMemory(self->device_context->logical_device ,
        self->image , vkmemory_block_get_device_memory(memory_block) ,
        vkmemory_block_get_offset(memory_block) + offset);
}

VkResult
wvkimage_init_aliased(
    _WVkImage *        self ,
    VkImageCreateInfo  image_info ,
    _VkMemoryBlock *   memory_block ,
    VkDeviceSize       offset ,
    VkExtent3D         extent ,
    VkFormat           format ,
    qo_uint32_t        mip_levels ,
    qo_uint32_t        array_layers
) {
    image_info.flags |= VK_IMAGE_CREATE_ALIAS_BIT;
    VkResult  res = vkCreateImage(memory_block->device_context->logical_device ,
        &image_info , get_vk_allocator() , &self->image);
    if (res != VK_SUCCESS)
    {
        return res;
    }
    res = wvkimage_bind_memory(self , memory_block , offset);
    if (res != VK_SUCCESS)
    {
        vkmemory_block_unref(memory_block);
        vkDestroyImage(memory_block->device_context->logical_device, self->image, get_vk_allocator());
        return res;
    }

    self->extent = extent;
    self->format = format;
    self->mip_levels = mip_levels;
    self->array_layers = array_layers;
    self->current_layout = image_info.initialLayout;
    self->device_context = memory_block->device_context;
}

// Factory function
VkResult
wvkimage_new(
    _WVkImage **                    p_self ,
    VkImageCreateInfo const *       create_info ,
    VkExtent3D                      extent ,
    VkFormat                        format ,
    qo_uint32_t                     mip_levels ,
    qo_uint32_t                     array_layers ,
    _VkDeviceContext *              device_context ,
    VmaAllocationCreateInfo const * alloc_info
) {
    _WVkImage * self = mi_malloc_tp(_WVkImage);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkimage_init_brandnew(
        self , create_info , extent , format , mip_levels , array_layers ,
        device_context , alloc_info
    );
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    *p_self = self;
}

VkResult
wvkimage_new_alised(
    _WVkImage **              p_self ,
    VkImageCreateInfo const * image_info ,
    _VkMemoryBlock *          memory_block ,
    VkDeviceSize              offset ,
    VkExtent3D                extent ,
    VkFormat                  format ,
    qo_uint32_t               mip_levels ,
    qo_uint32_t               array_layers
) {
    _WVkImage * self = mi_malloc_tp(_WVkImage);
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkimage_init_aliased(
        self , *image_info , memory_block , offset , extent , format ,
        mip_levels , array_layers
    );
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    *p_self = self;
}

void
wvkimage_destroy(
    _WVkImage * self
) {
    vkDestroyImage(self->device_context->logical_device, self->image, get_vk_allocator());
    vkmemory_block_unref(self->memory_block);
}

void
wvkimage_unref(
    _WVkImage * self
) {
    self->reference_count--;
    if (self->reference_count)
        return;
    wvkimage_destroy(self);
    mi_free(self);
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
// VkDescriptorImageInfo
// wvkimage_make_descriptor_info(
//     _WVkImage * self ,
//     VkSampler   sampler
// ) {
//     VkDescriptorImageInfo  descriptor_info = {
//         .imageLayout = self->current_layout ,
//         .imageView = self->default_view ,
//         .sampler = sampler
//     };
//     return descriptor_info;
// }


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
// VkResult
// wvkimage_create_default_view(
//     _WVkImage * self
// ) {
//     VkImageViewCreateInfo  view_info = {
//         .sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
//         .image  = self->image ,
//         .format = self->create_info.format ,
//         .components = {}
//     };
//     switch (self->create_info.imageType)
//     {
//         case VK_IMAGE_TYPE_1D:
//             view_info.viewType = (self->create_info.arrayLayers > 1) ?
//                                  VK_IMAGE_VIEW_TYPE_1D_ARRAY :
//                                  VK_IMAGE_VIEW_TYPE_1D;
//             break;

//         case VK_IMAGE_TYPE_2D:
//             if (self->create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
//             {
//                 view_info.viewType = (self->create_info.arrayLayers > 1) ?
//                                      VK_IMAGE_VIEW_TYPE_CUBE_ARRAY :
//                                      VK_IMAGE_VIEW_TYPE_CUBE;
//             }
//             else {
//                 view_info.viewType = (self->create_info.arrayLayers > 1) ?
//                                      VK_IMAGE_VIEW_TYPE_2D_ARRAY :
//                                      VK_IMAGE_VIEW_TYPE_2D;
//             }
//             break;

//         case VK_IMAGE_TYPE_3D:
//             view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
//             break;

//         default:
//             return INT32_MAX; // Unsupported image type for default view creation
//     }

//     if (self->create_info.format >= VK_FORMAT_D16_UNORM &&
//         self->create_info.format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
//     { // This is a depth/stencil format.
//         view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//         if (self->create_info.format == VK_FORMAT_D24_UNORM_S8_UINT)
//         {
//             view_info.subresourceRange.aspectMask |=
//                 VK_IMAGE_ASPECT_STENCIL_BIT;
//         }
//     }
//     else { // We assume it is a color format
//         view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     }

//     view_info.subresourceRange.baseMipLevel = 0;
//     view_info.subresourceRange.levelCount = self->mip_levels;
//     view_info.subresourceRange.baseArrayLayer = 0;
//     view_info.subresourceRange.layerCount = self->array_layers;

//     return vkCreateImageView(self->device_context->logical_device , &view_info ,
//         get_vk_allocator() , &self->default_view);
// }

// VkResult
// wvkimage_get_view1(
//     _WVkImage *                   self ,
//     VkImageViewCreateInfo const * view_info ,
//     VkImageView *                 p_view
// ) {
//     QO_ASSERT(view_info->image == self->image);

//     VkImageView found_view;
//     if (vk_image_view_map_search(&self->view_map, view_info, &found_view))
//     { // Cache hit
//         *p_view = found_view;
//         return VK_SUCCESS;
//     }

//     VkImageView new_view;
//     VkResult ret = vkCreateImageView(self->device_context->logical_device , view_info ,
// get_vk_allocator() , &new_view);
//     if (ret != VK_SUCCESS)
//     {
//         return ret;
//     }
//     vk_image_view_map_insert(&self->view_map, view_info, new_view);
//     *p_view = new_view;
//     return VK_SUCCESS;
// }

// VkResult
// wvkimage_get_view2(
//     _WVkImage *                     self ,
//     VkImageSubresourceRange const * subresource_range ,
//     VkImageViewType                 auto_or_user_type ,
//     VkImageView *                   p_view
// ) {
//     VkImageViewCreateInfo  create_info = {
//         .sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
//         .image  = self->image ,
//         .format = self->format ,
//         .subresourceRange = *subresource_range ,
//         .components = {}
//     };

//     if (auto_or_user_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM)
//     {
//         // TODO: Same as default view creation. Extract it out!
//     }
//     else
//     {
//         create_info.viewType = auto_or_user_type;
//     }

//     return wvkimage_get_view1(self, &create_info, p_view);

//     // if (self->array_layers > 1)
//     // {
//     //     //create_info.viewType = //(create_info.imageInfo.flags &
//     //     // VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? VK_IMAGE_VIEW_TYPE_CUBE :
//     //     // VK_IMAGE_VIEW_TYPE_2D_ARRAY;
//     // }
//     // else if (self->extent.height > 1)
//     // {
//     //     create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//     // }
//     // else {
//     //     create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
//     // }
// }

// VkResult
// wvkimage_generate_mipmaps(
//     _WVkImage * self ,
//     VkCommandBuffer command_buffer
// ) {
//     if (self->mip_levels <= 1)
//     {
//         // TODO: work out a return code
//     }


// }


