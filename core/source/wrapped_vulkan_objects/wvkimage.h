#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPED_VULKAN_IMAGE_SRC__
#include "../rendering_env.h"
#include "../vkmemory_block.h"
#include "../container/optional.h"
#include "../container/funnel_hash_table.h"
#include "../container/vkimage_view_map.h"
#include <xxh3.h>

struct __WVkImageDescription
{
    VkExtent3D             extent; //< for 2D image, `depth` is always 1
    VkFormat               format;
    VkImageUsageFlags      image_usage;
    VkImageType            type;
    VmaMemoryUsage         memory_usage;
    VmaAllocationCreateFlags allocation_flags;
    VkSampleCountFlagBits  samples;

    //< 0 means maximum possible mip levels, 1 means no mipmap generation
    qo_uint32_t    mip_levels;
    qo_uint32_t    array_layers;

    qo_ccstring_t  debug_name;

    _OPTIONAL(VkImageTiling , tiling);
    _OPTIONAL(VkSharingMode, sharing_mode);
    _OPTIONAL(VkImageLayout , initial_layout);
    _OPTIONAL(struct {
        qo_uint32_t       count;
        qo_uint32_t *     data;
    } , queue_family_indices);

};
typedef struct __WVkImageDescription _WVkImageDescription;

struct __WVkImage
{
    // Since _WVKimage is often transfered between objects (like pools)
    // We need to keep a reference count to manage it's lifetime.
    qo_ref_count_t        reference_count;
    _WVkImageDescription  image_desc;
    VkImage               image;

    // Not directly use VmaAllocation so we can easily creat alised
    // If it's NULL, this image is swapchain image
    _VkMemoryBlock *   memory_block;

    _VkDeviceContext * device_context;

    qo_uint32_t        max_mip_levels;

    qo_uint64_t        id;

    VkImageLayout      global_layout; // VK_IMAGE_LAYOUT_MAX_ENUM wiil be set if not shared
    qo_uint32_t        used_subresource_layout_count;
    VkImageLayout      subresource_layouts[]; //< max_mip_levels
};
typedef struct __WVkImage _WVkImage;

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)

qo_uint64_t
wvkimage_get_id(
    _WVkImage * self
) {
    return self->id;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
VkExtent3D
wvkimage_get_extent(
    _WVkImage * self
) {
    return self->image_desc.extent;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
VkFormat
wvkimage_get_format(
    _WVkImage * self
) {
    return self->image_desc.format;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
qo_uint32_t
wvkimage_get_mip_levels(
    _WVkImage * self
) {
    return self->image_desc.mip_levels;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
qo_uint32_t
wvkimage_get_array_layers(
    _WVkImage * self
) {
    return self->image_desc.array_layers;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
VkImageLayout
wvkimage_get_global_layout(
    _WVkImage * self
) {
    return self->global_layout;
}

QO_NODISCARD QO_NONNULL(1)
VkImageLayout
wvkimage_get_subresource_layout(
    _WVkImage *  self ,
    qo_uint32_t  mip_level ,
    qo_uint32_t  array_layer
);

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
_VkMemoryBlock *
wvkimage_get_memory_block(
    _WVkImage * self
) {
    return self->memory_block;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
VkImage
wvkimage_get_handle(
    _WVkImage * self
) {
    return self->image;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
_WVkImageDescription const *
wvkimage_get_image_info(
    _WVkImage * self
) {
    return &self->image_desc;
}

/// @brief Create a new wvkimage.
/// @param p_self Pointer to _WVkImage *
/// @param image_desc 
/// @param device_context 
/// @param alloc_info Set it NULL if not wanting. When it isn't NULL, `
///                   `allocation_flags` and `memory_usage` in `image_desc` will
///                   be ignored.
/// @return VkResult 
QO_NODISCARD 
VkResult
wvkimage_new(
    _WVkImage **                    p_self ,
    _WVkImageDescription *          image_desc ,
    _VkDeviceContext *              device_context ,
    VmaAllocationCreateInfo const * alloc_info
);

QO_NONNULL(1 , 2)
VkResult
wvkimage_bind_memory(
    _WVkImage *      self ,
    _VkMemoryBlock * memory_block ,
    VkDeviceSize     size
);

QO_NONNULL(1 , 2 , 3)
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

QO_NODISCARD QO_NONNULL(1)
VkResult
wvkimage_set_current_layout(
    _WVkImage *    self ,
    VkImageLayout  new_layout
);

// This shouldn't be called when the image needs transition of queue ownership
QO_NONNULL(1 , 2)
void
wvkimage_simple_record_transition(
    _WVkImage *      self ,
    VkCommandBuffer  command_buffer ,
    VkImageLayout    new_layout
);

QO_NODISCARD QO_NONNULL(1 , 2 , 3)
VkResult
wvkimage_record_transition(
    _WVkImage *                  self ,
    VkCommandBuffer              command_buffer ,
    VkImageMemoryBarrier const * user_barrier
);

void
wvkimage_unref(
    _WVkImage * self
);

// // // simple path, but lack of customization
// // // We should let expert to do those underlying.
// // VkResult
// // wvkimage_simple_create(
// //     _WVkImage *                     self ,
// //     VkImageCreateInfo const *       image_info ,
// //     VmaAllocationCreateInfo const * alloc_info ,
// //     VkFormat                        format ,
// //     VkExtent3D                      extent ,
// //     VkDevice                        device
// // ) {
// //     self->extent = extent;
// //     self->format = format;
// //     VkResult  ret = vmaCreateImage(
// //         g_vk_global_context.allocator , image_info , alloc_info , &self->image ,
// //         &self->allocation , NULL
// //     );
// //     if (ret != VK_SUCCESS)
// //     {
// //         return ret;
// //     }

// //     // TODO: Make it customizable, esp. `components` and `subresourceRange`
// //     VkImageViewCreateInfo  view_info = {
// //         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ,
// //         .pNext = NULL ,
// //         .flags = 0 ,
// //         .image = self->image ,
// //         .viewType = VK_IMAGE_VIEW_TYPE_2D ,
// //         .format = format ,
// //         .components = {
// //             .r = VK_COMPONENT_SWIZZLE_IDENTITY ,
// //             .g = VK_COMPONENT_SWIZZLE_IDENTITY ,
// //             .b = VK_COMPONENT_SWIZZLE_IDENTITY ,
// //             .a = VK_COMPONENT_SWIZZLE_IDENTITY ,
// //         } ,
// //         .subresourceRange = {
// //             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ,
// //             .baseMipLevel = 0 ,
// //             .levelCount = 1 ,
// //             .baseArrayLayer = 0 ,
// //             .layerCount = 1 ,
// //         }
// //     };
// // }
