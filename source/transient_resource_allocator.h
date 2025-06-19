#pragma once
#define __QOR_TRANSIENT_RESOURCE_ALLOCATOR_SRC__

#include "wrapped_vulkan_image.h"
#include "string_hash_table.h"
#include <xxhash.h>

struct __TransientImageRequest
{
    XXH64_hash_t       logical_name_hash;
    VkImageCreateInfo  create_info;
    struct
    {
        qo_uint32_t  first_use;
        qo_uint32_t  last_use;
    } pass_index;
};
typedef struct __TransientImageRequest _TransientImageRequest;

// Manage resources that endure within a frame
struct __TransientImageAllocator
{
    
};
typedef struct __TransientImageAllocator _TransientImageAllocator;

VkResult
plan_transient_frame(
    _TransientImageAllocator * allocator ,
    _TransientImageRequest *   requests ,
    qo_uint32_t                   request_count
);

_WVkImage *
get_transient_frames(
    _TransientImageAllocator * allocator ,
    XXH64_hash_t                 logical_name_hash
);

// Call this after all rendering done
void
reclaim_transient_frames(
    _TransientImageAllocator * allocator 
);

