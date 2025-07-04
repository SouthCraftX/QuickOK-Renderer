#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WRAPED_VULKAN_IMAGE_SRC__
#include "../rendering_env.h"
#include "../vkmemory_block.h"
#include "../container/funnel_hash_table.h"
#include "../container/vkimage_view_map.h"
#include <xxh3.h>
struct __WVkImage
{
    // Since _WVKimage is often transfered between objects (like pools)
    // We need to keep a reference count to manage it's lifetime.
    qo_ref_count_t     reference_count;
    VkImageCreateInfo  image_info;
    VkImage            image;
    VkExtent3D         extent;     // For 2D image, `depth` is always 1
    VkFormat           format;
    _VkMemoryBlock *   memory_block; // Not directly use VmaAllocation so we can easily creat alised
    qo_uint32_t        mip_levels;
    qo_uint32_t        array_layers;
    VkImageLayout      current_layout;
    qo_uint64_t        id; 
    // VkImageCreateInfo  image_info;
    // XXH64_hash_t       image_info_hash;
    _VkDeviceContext * device_context;
};
typedef struct __WVkImage _WVkImage; 

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
qo_uint64_t
wvkimage_get_id(
    _WVkImage * self
) {
    return self->id;
}

QO_NODISCARD QO_NONNULL(1)
VkExtent3D
wvkimage_get_extent(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
VkFormat
wvkimage_get_format(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
qo_uint32_t
wvkimage_get_mip_levels(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
qo_uint32_t
wvkimage_get_array_layers(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
VkImageLayout
wvkimage_get_current_layout(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
_VkMemoryBlock *
wvkimage_get_memory_block(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
VkImage
wvkimage_get_handle(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1)
VkImageCreateInfo const *
wvkimage_get_create_info(
    _WVkImage * self
);

QO_NODISCARD QO_NONNULL(1 , 2 , 7 , 8)
VkResult
wvkimage_new(
    _WVkImage **                    p_self ,
    VkImageCreateInfo const * create_info ,
    VkExtent3D                extent ,
    VkFormat                  format ,
    qo_uint32_t               mip_levels ,
    qo_uint32_t               array_layers ,
    _VkDeviceContext *       device_context ,
    VmaAllocationCreateInfo const * alloc_info
);

QO_NONNULL(1 , 2)
VkResult
wvkimage_bind_memory(
    _WVkImage *         self ,
    _VkMemoryBlock *    memory_block ,
    VkDeviceSize        size
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
