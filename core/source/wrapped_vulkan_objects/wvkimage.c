#include "wvkimage.h"
#include "../access_info_inferor.h"
#include "../rendering_env.h"
#include "../command_buffer.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#include <math.h>
#include <xxh3.h>

QO_NODISCARD QO_NO_SIDE_EFFECTS

qo_uint32_t
calc_max_mip_levels(
    VkExtent3D  extent
) {
    return floor(log2(fmax(extent.width , extent.height))) + 1;
}

QO_NODISCARD

qo_size_t
calc_subresource_layout_array_count(
    _WVkImageDescription const * image_desc ,
    qo_uint32_t *                p_final_mip_levels
) {
    const qo_uint32_t  max_mip_levels =
        calc_max_mip_levels(image_desc->extent);
    const qo_bool_t    generate_mipmap = image_desc->mip_levels != 1;
    if (generate_mipmap)
    {
        if (!image_desc->mip_levels || image_desc->mip_levels > max_mip_levels)
        {
            *p_final_mip_levels = max_mip_levels;
        }
    }
    return image_desc->array_layers * image_desc->mip_levels;
}

VkImageLayout
wvkimage_get_layout(
    _WVkImage const * self ,
    qo_uint32_t       mip_layer ,
    qo_uint32_t       array_layer
) {
}

VkImageSubresourceRange
create_full_subresource_range(
    _WVkImage *  self ,
    qo_uint32_t  mip_levels ,
    qo_uint32_t  array_layers
) {
    VkImageSubresourceRange  range = {
        .baseMipLevel = 0 ,
        .levelCount = mip_levels ,
        .baseArrayLayer = 0 ,
        .layerCount = array_layers ,
    };
    VkImageCreateInfo const * vkimage_info = &self->image_info.vkimage_info;
    if (vkimage_info->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (vkimage_info->format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            vkimage_info->format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    return range;
}

void
wvkimage_set_current_layout(
    _WVkImage *                     self ,
    VkImageLayout                   new_layout ,
    VkImageSubresourceRange const * range
) {
    qo_bool_t  all_identical = qo_true;
    for (qo_uint32_t layer = range->baseArrayLayer ;
         layer < range->baseArrayLayer + range->layerCount ; ++layer)
    {
        for (qo_uint32_t mip = range->baseMipLevel ;
             mip < range->baseMipLevel + range->levelCount ; ++mip)
        {
            self->subresource_layouts[layer * self->max_mip_levels +
                                      mip] = new_layout;
        }
    }

    VkImageLayout  first_layout = self->subresource_layouts[0];
    for (qo_uint32_t i = 1 ; i < self->used_layouts ; ++i)
    {
        if (self->subresource_layouts[i] != first_layout)
        {
            all_identical = qo_false;
            break;
        }
    }

    self->global_layout =
        all_identical ? first_layout : VK_IMAGE_LAYOUT_MAX_ENUM;
}

#define ASSIGN_IF_HAS_VALUE(x , y , d) if (y.has_value) { x = y.data; } else { x = d;}

void
make_vkimage_create_info(
    _WVkImageDescription const * image_desc ,
    VkImageCreateInfo *          p_vkimage_create_info
) {
    p_vkimage_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    p_vkimage_create_info->pNext = NULL;
    p_vkimage_create_info->imageType = image_desc->type;
    p_vkimage_create_info->arrayLayers = image_desc->array_layers;
    p_vkimage_create_info->mipLevels = image_desc->mip_levels;
    p_vkimage_create_info->format  = image_desc->format;
    p_vkimage_create_info->usage   = image_desc->image_usage;
    p_vkimage_create_info->samples = image_desc->samples;
    ASSIGN_IF_HAS_VALUE(p_vkimage_create_info->tiling, image_desc->tiling, VK_IMAGE_TILING_OPTIMAL);
    ASSIGN_IF_HAS_VALUE(p_vkimage_create_info->sharingMode, image_desc->sharing_mode, VK_SHARING_MODE_EXCLUSIVE);
    ASSIGN_IF_HAS_VALUE(p_vkimage_create_info->initialLayout, image_desc->initial_layout, VK_IMAGE_LAYOUT_UNDEFINED)
    if (image_desc->queue_family_indices.has_value)
    {
        p_vkimage_create_info->queueFamilyIndexCount = image_desc->queue_family_indices.data.count;
        p_vkimage_create_info->pQueueFamilyIndices = image_desc->queue_family_indices.data.data;
    }
    else 
    {
        p_vkimage_create_info->queueFamilyIndexCount = 0;
        p_vkimage_create_info->pQueueFamilyIndices = NULL;
    }
}

#undef ASSIGN_IF_HAS_VALUE

VkResult
wvkimage_init_brandnew(
    _WVkImage *                     self ,
    _WVkImageDescription const *    user_image_desc ,
    _VkDeviceContext *              device_context ,
    VmaAllocationCreateInfo const * alloc_info ,
    qo_uint32_t                     actual_mip_levels
) {
    VkImageCreateInfo  image_info;
    make_vkimage_create_info(user_image_desc , &image_info);
    if (actual_mip_levels > 1)
    {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkResult  res = vkCreateImage(self->device_context->logical_device ,
        &image_info , get_vk_allocator() , &self->image);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    VmaAllocationCreateInfo  allocation_info;
    if (!alloc_info)
    {
        memset(&allocation_info , 0 , sizeof(VmaAllocationCreateInfo));
        allocation_info.usage = user_image_desc->memory_usage;
        allocation_info.flags = user_image_desc->allocation_flags;
        alloc_info = &allocation_info;
    }

    _VkMemoryBlock * memory_block;
    res = vkmemory_block_new(&memory_block , device_context ,
        &device_context->memory_requirements.image , *alloc_info);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    self->image_desc = *user_image_desc;
    self->image_desc.mip_levels = actual_mip_levels;
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
        vkDestroyImage(memory_block->device_context->logical_device ,
            self->image , get_vk_allocator());
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
    _WVkImageDescription const *    image_desc ,
    _VkDeviceContext *              device_context ,
    VmaAllocationCreateInfo const * alloc_info
) {
    qo_uint32_t  mip_levels = image_desc->mip_levels;
    qo_size_t    layout_count = calc_subresource_layout_array_count(image_desc ,
        &mip_levels);
    _WVkImage *  self = mi_malloc(
        sizeof(_WVkImage) + layout_count *
        sizeof(VkImageLayout)
    );
    if (!self)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult  ret = wvkimage_init_brandnew(
        self , image_desc , device_context , alloc_info , mip_levels
    );
    if (ret != VK_SUCCESS)
    {
        mi_free(self);
        return ret;
    }
    *p_self = self;
    return VK_SUCCESS;
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
    vkDestroyImage(self->device_context->logical_device , self->image ,
        get_vk_allocator());
    vkmemory_block_unref(self->memory_block);
}

void
wvkimage_unref(
    _WVkImage * self
) {
    self->reference_count--;
    if (self->reference_count)
    {
        return;
    }
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
    if (self->current_layout == new_layout)
    {
        return;
    }

    _LegacyBarrierAccessInfo  source_info =
        infer_legacy_access_info(self->current_layout);
    _LegacyBarrierAccessInfo  target_info =
        infer_legacy_access_info(new_layout);

    _ImageTransition  transition = {
        .image = self ,
        .old_layout = self->current_layout ,
        .new_layout = new_layout ,
        .subresource_range.has_value = qo_false ,
        .source_queue_family = VK_QUEUE_FAMILY_IGNORED ,
        .target_queue_family = VK_QUEUE_FAMILY_IGNORED
    };
}

void
wvkimage_record_color_clear(
    _WVkImage *                     self ,
    VkCommandBuffer                 command_buffer ,
    VkClearColorValue const *       color_value ,
    VkImageSubresourceRange const * ranges ,
    qo_uint32_t                     range_count
) {
    vkCmdClearColorImage(command_buffer , self->image , self->current_layout ,
        color_value , range_count , ranges);
}

void
wvkimage_record_depth_clear(
    _WVkImage *                      self ,
    VkCommandBuffer                  command_buffer ,
    VkClearDepthStencilValue const * clear_value ,
    VkImageSubresourceRange const *  ranges ,
    qo_uint32_t                      range_count
) {
    vkCmdClearDepthStencilImage(command_buffer , self->image ,
        self->current_layout , clear_value , range_count , ranges);
}

void
wvkimage_simple_record_color_clear(
    _WVkImage *               self ,
    VkCommandBuffer           command_buffer ,
    VkClearColorValue const * color_value
) {
    VkImageSubresourceRange  full_range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ,
        .baseMipLevel = 0 ,
        .levelCount = self->mip_levels ,
        .baseArrayLayer = 0 ,
        .layerCount = self->array_layers
    };
    wvkimage_record_color_clear(self , command_buffer , color_value ,
        &full_range , 1);
}

void
wvkimage_simple_record_depth_clear(
    _WVkImage *                      self ,
    VkCommandBuffer                  command_buffer ,
    VkClearDepthStencilValue const * clear_value
) {
    VkImageSubresourceRange  full_range = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT ,
        .baseMipLevel = 0 ,
        .levelCount = self->mip_levels ,
        .baseArrayLayer = 0 ,
        .layerCount = self->array_layers
    };
    if (self->image_info.format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        self->image_info.format == VK_FORMAT_D24_UNORM_S8_UINT)
    {
        full_range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    wvkimage_record_depth_clear(self , command_buffer , clear_value ,
        &full_range , 1);
}

VkResult
wvkimage_record_copy(
    _WVkImage *      source_image ,
    _WVkImage *      target_image ,
    VkCommandBuffer  command_buffer
) {
    VkImageLayout  original_source_layout = source_image->current_layout;
    VkImageLayout  original_target_layout = target_image->current_layout;

    _ImageTransition  source_transition = {
        .image = source_image ,
        .old_layout = original_source_layout ,
        .new_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    };
    _ImageTransition  target_transition = {
        .image = target_image ,
        .old_layout = original_target_layout ,
        .new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    };

    _LegacyBarrierAccessInfo  source_info_x =
        infer_legacy_access_info(original_source_layout);
    _LegacyBarrierAccessInfo  source_info_y =
        infer_legacy_access_info(original_target_layout);
    _LegacyBarrierAccessInfo  target_info_transfer =
        infer_legacy_access_info(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

VkResult
wvkimage_record_mipmap_generation(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer
) {
#if defined (DEBUG)
    // Check liner filter
#endif
    VkImageSubresourceRange  mip0_range = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ,
        .baseMipLevel = 0 ,
        .levelCount = 1 ,
        .baseArrayLayer = 0 ,
        .layerCount = self->array_layers
    };

    // TODO: Record transition

    qo_int32_t  mip_width  = self->extent.width;
    qo_int32_t  mip_height = self->extent.height;

    //for (qo_uint32_t i = 1 ; self->)
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
// )
