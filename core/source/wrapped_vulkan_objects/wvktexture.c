#include "wvktexture.h"
#include "wvkimage.h"
#include <vulkan/vulkan_core.h>

VkDescriptorImageInfo
wvktexture_get_descriptor_info(
    _WVkTexture * self ,
    VkImageView   view
) {
    VkDescriptorImageInfo  info = {
        .sampler = self->sampler ,
        .imageView = view ,
        .imageLayout = wvkimage_get_current_layout(self->image)
    };
    return info;
}

QO_GLOBAL_LOCAL QO_FORCE_INLINE
qo_uint64_t
make_wvktexture_hash(
    _WVkImage *  image ,
    qo_uint64_t  sampler_id
) {
    qo_uint64_t  id[] = {
        wvkimage_get_id(image) ,
        sampler_id
    };
    return XXH3_64bits(id , sizeof(id));
}

VkResult
wvktexture_get_view(
    _WVkTexture *          self ,
    VkImageViewCreateInfo  partial_view_info ,
    VkImageView *          p_view
) {
    partial_view_info.image = wvkimage_get_handle(self->image);
    if (vkimage_view_map_search(&self->custom_views_map , &partial_view_info ,
        p_view))
    {
        return VK_SUCCESS;
    }
    VkImageView  new_view;
    VkResult  ret = vkCreateImageView(self->device_context->logical_device ,
        &partial_view_info , get_vk_allocator() , &new_view);
    if (ret != VK_SUCCESS)
    {
        return ret;
    }
    if (vkimage_view_map_insert(&self->custom_views_map , &partial_view_info ,
        new_view))
    {
        *p_view = new_view;
        return VK_SUCCESS;
    }
    vkDestroyImageView(self->device_context->logical_device , new_view ,
        get_vk_allocator());
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

VkResult
wvktexture_get_view2(
    _WVkTexture *                   self ,
    VkImageSubresourceRange const * subresource_range ,
    VkImageViewType                 view_type ,
    VkImageView *                   p_view
) {
    VkImageViewCreateInfo  view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
        .pNext = NULL ,
        .image = wvkimage_get_handle(self->image) ,
        .flags = 0
    };
    VkImageCreateInfo const * image_info =
        wvkimage_get_create_info(self->image);
    view_info.format = image_info->format;
    view_info.subresourceRange = *subresource_range;

    view_info.components = (VkComponentMapping) {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY ,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY ,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY ,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY ,
    };

    // Make it convenient for following decision
    subresource_range = &view_info.subresourceRange;

    if (view_type != VK_IMAGE_VIEW_TYPE_MAX_ENUM)
    {
        view_info.viewType = view_type;
    }
    else {
        qo_bool_t  is_array = subresource_range->layerCount > 1;
        switch (image_info->imageType)
        {
            case VK_IMAGE_TYPE_1D:
                view_info.viewType =
                    is_array ? VK_IMAGE_VIEW_TYPE_1D_ARRAY :
                    VK_IMAGE_VIEW_TYPE_1D;
                break;

            case VK_IMAGE_TYPE_2D:
                if (image_info->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
                {
                    view_info.viewType = (subresource_range->layerCount >
                        6) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY :
                                         (is_array ? VK_IMAGE_VIEW_TYPE_CUBE :
                                             VK_IMAGE_VIEW_TYPE_2D);
                }
                else {
                    view_info.viewType =
                        is_array ? VK_IMAGE_VIEW_TYPE_2D_ARRAY :
                        VK_IMAGE_VIEW_TYPE_2D;
                }
                break;

            case VK_IMAGE_TYPE_3D:
                view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;

            default:
                return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }
}

VkResult
wvktexture_create_default_view(
    _WVkTexture * self
) {
    VkImageCreateInfo const * image_info =
        wvkimage_get_create_info(self->image);
    VkImageSubresourceRange   full_range = {};
    if (image_info->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        full_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (image_info->format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            image_info->format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            full_range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        full_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    full_range.baseMipLevel = 0;
    full_range.levelCount = image_info->mipLevels;
    full_range.baseArrayLayer = 0;
    full_range.layerCount = image_info->arrayLayers;
    return wvktexture_get_view2(self->image , &full_range ,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM , &self->default_view);
}

VkResult
wvktexture_get_default_view(
    _WVkTexture * self ,
    VkImageView *   p_view
) {
    if (self->default_view == VK_NULL_HANDLE)
    {
        VkResult ret = wvktexture_create_default_view(self);
        if (ret != VK_SUCCESS)
        {
            return ret;
        }
    }
    *p_view = self->default_view;
    return VK_SUCCESS;

}

void
wvktexture_destroy(
    _WVkTexture *   self
) {
    wvkimage_unref(self->image);
    
    _FunnelHashTableIterator iterator = fht_iterate(&self->custom_views_map);
    VkImageView view;
    while (fht_iterator_next(&self->custom_views_map, NULL , &view))
    {
        vkDestroyImageView(self->device_context->logical_device , view ,
            get_vk_allocator());
    }
    if (self->default_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(self->device_context->logical_device ,
            self->default_view , get_vk_allocator());
    }
    // We don't destroy VkSampler here, which is owned by _SamplerCache
}

void
wvktexture_unref(
    _WVkTexture *   self
) {
    self->refernce_count--;
    if (self->refernce_count)
    {
        return;
    }
    wvktexture_destroy(self);
}