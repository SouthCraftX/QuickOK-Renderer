#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPED_VULKAN_IMAGE_SRC__

#include "rendering_env.h"
#include "mimalloc_vulkan_callback.h"
#include "funnel_hash_table.h"
#include "vk_image_view_map.h"
#include <xxh3.h>

struct __WVkImage
{
    // Since _WVKimage is often transfered between objects (like pools)
    // We need to keep a reference count to manage it's lifetime.
    qo_ref_count_t     reference_count;
    VkImage            image;
    VkImageView        default_view;
    VkExtent3D         extent;     // For 2D image, `depth` is always 1
    VkFormat           format;
    VmaAllocation      allocation;
    qo_uint32_t        mip_levels;
    qo_uint32_t        array_layers;
    VkImageLayout      current_layout;
    VkImageCreateInfo  create_info;
    _VkDeviceContext * device_context;
    _VkImageViewMap    view_map;
};
typedef struct __WVkImage _WVkImage;
VkResult
wvkimage_new(
    _WVkImage **                    p_self ,
    _VkDeviceContext *              device_context ,
    VkImageCreateInfo const *       create_info ,
    VmaAllocationCreateInfo const * alloc_info ,
    qo_bool_t                       create_default_view
);

VkResult
wvkimage_allocate_memory(
    _WVkImage *                     self ,
    VmaAllocationCreateInfo const * alloc_info
);

VkResult
wvkimage_create_default_view(
    _WVkImage * self
);

VkResult
wvkimage_generate_mipmaps(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer
);

VkResult
wvkimage_clear(
    _WVkImage *               self ,
    VkCommandBuffer           command_buffer ,
    VkClearColorValue const * clear_color
);

VkResult
wvkimage_copy(
    _WVkImage *      source ,
    _WVkImage *      destination ,
    VkCommandBuffer  command_buffer
);

VkResult
wvkimage_set_current_layout(
    _WVkImage *    self ,
    VkImageLayout  new_layout
);

// This shouldn't be called when the image needs transition of queue ownership
VkResult
wvkimage_simple_record_transition(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer ,
    VkImageLayout    new_layout
);

VkResult
wvkimage_record_transition(
    _WVkImage *                  self ,
    VkCommandBuffer              command_buffer ,
    VkImageMemoryBarrier const * user_barrier
);

VkDescriptorImageInfo
wvkimage_get_descriptor_info(
    _WVkImage * self ,
    VkSampler   sampler
);

void
wvkimage_unref(
    _WVkImage * self
);

// // simple path, but lack of customization
// // We should let expert to do those underlying.
// VkResult
// wvkimage_simple_create(
//     _WVkImage *                     self ,
//     VkImageCreateInfo const *       image_info ,
//     VmaAllocationCreateInfo const * alloc_info ,
//     VkFormat                        format ,
//     VkExtent3D                      extent ,
//     VkDevice                        device
// ) {
//     self->extent = extent;
//     self->format = format;
//     VkResult  ret = vmaCreateImage(
//         g_vk_global_context.allocator , image_info , alloc_info , &self->image ,
//         &self->allocation , NULL
//     );
//     if (ret != VK_SUCCESS)
//     {
//         return ret;
//     }

//     // TODO: Make it customizable, esp. `components` and `subresourceRange`
//     VkImageViewCreateInfo  view_info = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
//         .pNext = NULL ,
//         .flags = 0 ,
//         .image = self->image ,
//         .viewType = VK_IMAGE_VIEW_TYPE_2D ,
//         .format = format ,
//         .components = {
//             .r = VK_COMPONENT_SWIZZLE_IDENTITY ,
//             .g = VK_COMPONENT_SWIZZLE_IDENTITY ,
//             .b = VK_COMPONENT_SWIZZLE_IDENTITY ,
//             .a = VK_COMPONENT_SWIZZLE_IDENTITY ,
//         } ,
//         .subresourceRange = {
//             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ,
//             .baseMipLevel = 0 ,
//             .levelCount = 1 ,
//             .baseArrayLayer = 0 ,
//             .layerCount = 1 ,
//         }
//     };
// }
