#pragma once
#define __QOR_TRANSIENT_IMAGE_ALLOCATOR_SRC__
#include "rendering_env.h"
#include "vkimage_info_hash_wvkimage_list_map.h"
#include <vulkan/vulkan_core.h>

#include "string_hash_index_map.h"
#include "wrapped_vulkan_image.h"
#include "vkimage_info_hash_wvkimage_list_map.h"
#include <xxhash.h>

struct __TransientImageDescription
{
    XXH64_hash_t  logical_name_hash;
    struct
    {
        VkImageCreateInfo  self;
        XXH64_hash_t       hash;
    } image_info;
    struct
    {
        qo_uint32_t  first_use;
        qo_uint32_t  last_use;
    } pass_index;
};
typedef struct __TransientImageDescription _TransientImageDescription;



// Manage resources that endure within a frame
struct __TransientImageAllocator
{
    struct
    {
        _WVkImage ** array;
        qo_size_t    used_count;
        qo_size_t    total_count;
    } active_images;
    _StringHash2WVkImageIndexMap      image_index_map;

    _VkDeviceContext *                device_context;

    _VkImageInfoHash2WVkImageListMap  free_image_list_map;

    // How many frames this allocator has helped
    qo_uint32_t                       generation;

    qo_uint64_t                       current_frame;
    // Range: 0 to MAX_FRAMES_IN_FLIGHT - 1
    // To track the final time when the resources are used
};
typedef struct __TransientImageAllocator _TransientImageAllocator;
VkResult
plan_transient_frame(
    _TransientImageAllocator *   self ,
    _TransientImageDescription * requests ,
    qo_uint32_t                  request_count
);

_WVkImage *
get_transient_frames(
    _TransientImageAllocator * self ,
    XXH64_hash_t               logical_name_hash
);

// Call this after all rendering done
qo_bool_t
reclaim_transient_frame_resources(
    _TransientImageAllocator * self
);
