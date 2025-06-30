#include "../wrapped_vulkan_objects/wvkbuffer.h"
#include "../container/staging_allocation_deque.h"
#include "../container/vector.h"
#include <vulkan/vulkan_core.h>

#define DEFAULT_STAGING_MANAGER_SIZE  (64 << 20) // 64MB

struct __StagingManager
{
    _VkDeviceContext *       device_context;
    VkDeviceSize             size;
    VkDeviceSize             head;
    VkDeviceSize             tail;
    _WVkBuffer *             ring_buffer;
    _StagingAllocationDeque  submitted_allocations;
    _Vector                  pending_frame_allocations;
};
typedef struct __StagingManager _StagingManager;

void
staging_manager_begin_frame(
    _StagingManager * self ,
    qo_uint64_t       completion_fence_value
);

QO_NODISCARD
VkResult
staging_manager_allocate(
    _StagingManager * self ,
    VkDeviceSize      size ,
    VkDeviceSize      alignment ,
    qo_pointer_t *    p_data ,
    VkBuffer *        p_vkbuffer ,
    VkDeviceSize *    p_offset
);

QO_NODISCARD
VkResult
staging_manager_submit(
    _StagingManager * self ,
    qo_uint64_t       fence_value
);
