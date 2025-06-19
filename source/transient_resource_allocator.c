#include "transient_resource_allocator.h"
#include "rendering_env.h"
#include "wvkimage_list.h"
#include <vulkan/vulkan_core.h>
// We temporarily disable 
// #define ARRAY_TO_LIST_THRESHOLD 8

struct __AllocationBlock
{
    VmaAllocation   allocation;
    VkDeviceSize    size;
    qo_uint32_t     free_from_pass_index;
};
typedef struct __AllocationBlock _AllocationBlock;
