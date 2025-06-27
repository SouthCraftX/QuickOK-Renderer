#include "transient_image_allocator.h"
#include "rendering_env.h"
#include "string_hash_index_map.h"
#include "transient_image_allocator.h"
#include "vkimage_info_hash_wvkimage_list_map.h"
#include "vulkan_make_debug_message.h"
#include "wrapped_vulkan_image.h"
#include "wrapped_vulkan_image_list.h"
#include <vulkan/vulkan_core.h>

_WVkImage *
acquire_from_list(
    _TransientImageAllocator * self ,
    _VkImageInfoCompositeKey * key
) {
}

qo_bool_t
release_to_list(
    _TransientImageAllocator * self ,
    _WVkImage *                image ,
    _VkImageInfoCompositeKey * info_key
) {
    _WVkImageList * list;
    if (vkimage_info_hash_wvkimage_list_map_search(&self->free_image_list_map ,
        info_key , &list))
    {
        return wvkimage_list_push_back(list , image);
    }
    else
    {
        list = wvkimage_list_new();
        if (!list)
        {
            return qo_false;
        }

        if (!wvkimage_list_push_back(list , image))
        {
            return qo_false;
        }

        return vkimage_info_hash_wvkimage_list_map_set(
            &self->free_image_list_map , info_key , list
        );
    }
}

qo_bool_t
place_active_images_array(
    _TransientImageAllocator * self ,
    _WVkImage *                image
) {
    if (self->active_images.total_count == self->active_images.used_count)
    {
        _WVkImage ** new_array = mi_reallocn_tp(self->active_images.array ,
            _WVkImage * , self->active_images.used_count * 2);
        if (!new_array)
        {
            return qo_false;
        }
        self->active_images.array = new_array;
        self->active_images.total_count *= 2;
    }
    self->active_images.array[self->active_images.used_count] = image;
    ++self->active_images.used_count;
    return qo_true;
}

VkResult
plan_transient_frame(
    _TransientImageAllocator *   self ,
    _TransientImageDescription * requests ,
    qo_uint32_t                  request_count
) {
    if (QO_UNLIKELY(!self->active_images.used_count))
    { // It's usually a logical error, but we can force to reclaim
        vulkan_send_debug_message(
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ,
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ,
            "Active images of transient image allocator is empty, but a request is made. "
            "This is usually a logical error, but we're trying to force to reclaim."
        );
        reclaim_transient_frame_resources(self);
    }

    for (qo_uint32_t i = 0; i < request_count; ++i)
    {
        _VkImageInfoCompositeKey  key = {
            .precomputed_hash = requests[i].image_info.hash ,
            .p_info = &requests[i].image_info.self ,
        };
        _WVkImage * image = acquire_from_list(self , &key);

        if (!image)
        {
            VmaAllocationCreateInfo  alloc_info = {};
            alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            VkResult  res = wvkimage_new(&image , self->device_context ,
                &requests[i].image_info.self , &alloc_info , qo_true);
            // TODO: Create default view? (We previously use qo_true)
            if (res != VK_SUCCESS)
            {
                return res;
            }
        }

        if (!place_active_images_array(self , image))
        {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }
        string_hash_wvkimage_index_map_set(&self->image_index_map ,
            requests[i].logical_name_hash , 
            (_ActiveImageIndexEntry) { 
                .index = self->active_images.used_count - 1 ,
                .generation = self->generation
            }
        );
    }
}

_WVkImage *
get_transient_image(
    _TransientImageAllocator * self ,
    string_hash_t              name_hash
) {
    _ActiveImageIndexEntry  index;
    qo_bool_t got =  string_hash_wvkimage_index_map_search(&self->image_index_map ,
        name_hash , &index);
    if (got)
    {
        if (index.generation == self->generation)
        {
            return self->active_images.array[index.index];
        }
    }
    return NULL;
// #if defined (DEBUG)
//     if (!success)
//     {
//         char message[128];
//         sprintf(message , "Failed to find image with name hash %llu." , name_hash);
//         vulkan_send_debug_message(
//             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ,
//             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ,
//             message
//         );
//     }
//     // TODO: abort and dump stack
// #endif // DEBUG
}

qo_bool_t
reclaim_transient_frame_resources(
    _TransientImageAllocator *  self
) {
    for (qo_uint32_t i = 0; i < self->active_images.used_count; ++i)
    {
        _WVkImage * image = self->active_images.array[i];
        _VkImageInfoCompositeKey  key = {
            .precomputed_hash = image->image_info_hash ,
            .p_info = &image->image_info
        };
        if (!release_to_list(self , image , &key))
        {
            return qo_false;
        }
    }
    self->active_images.used_count = 0;
    self->generation++;
    //vkimage_info_hash_wvkimage_list_map_destroy(_VkImageInfoHash2WVkImageListMap *self)
}