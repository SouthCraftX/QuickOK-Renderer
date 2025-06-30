#include "staging_manager.h"

void
staging_manager_begin_frame(
    _StagingManager * self ,
    qo_uint64_t       completion_fence_value
) {
    _StagingAllocation  allocation;
    while (!sadeque_is_empty(&self->submitted_allocations) &&
           (sadeque_pop_front(&self->submitted_allocations , &allocation) ,
            allocation.fence_value <= completion_fence_value)
    );
}

VkResult
staging_manager_allocate(
    _StagingManager * self ,
    VkDeviceSize      size ,
    VkDeviceSize      alignment ,
    qo_pointer_t *    p_data ,
    VkBuffer *        p_vkbuffer ,
    VkDeviceSize *    p_offset
) {
    if (size > self->size)
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    _StagingAllocation  allocation;
    sadeque_peek_front(&self->submitted_allocations , &allocation);
    const VkDeviceSize  aligned_head = (self->head + alignment - 1) &
                                       ~(alignment - 1);
    const VkDeviceSize  tail =
        sadeque_is_empty(&self->submitted_allocations) ? 0 : allocation.offset;

    if (aligned_head >= tail)
    {
        if (aligned_head + size > self->size)
        {
            if (size > tail)
            {
                return VK_ERROR_OUT_OF_DEVICE_MEMORY;
            }
            *p_offset  = 0;
            self->head = size;
        }
        else
        {
            *p_offset  = aligned_head;
            self->head = aligned_head + size;
        }
    }
    else {
        if (aligned_head + size > tail)
        {
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
        *p_offset  = aligned_head;
        self->head = aligned_head + size;
    }

    *p_vkbuffer = wvkbuffer_get_handle(self->ring_buffer);
    *p_data = (qo_byte_t *) (wvkbuffer_get_data(self->ring_buffer) + *p_offset);
    return vector_push_back(
        &self->pending_frame_allocations , &(_StagingAllocation) {
        UINT64_MAX , *p_offset , size
    }
    ) ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY;
}

VkResult
staging_manager_submit(
    _StagingManager * self ,
    qo_uint64_t       fence_value
) {
    _StagingAllocation  allocation;
    for (_VectorIterator now =
             vector_iterate_begin(&self->pending_frame_allocations) ,
         end = vector_iterate_end(&self->pending_frame_allocations);
         !vector_iterator_equals(&now , &end);
         vector_iterator_next(&now))
    {
        allocation = *(const _StagingAllocation *) vector_iterator_get(&now);
        allocation.fence_value = fence_value;
        if (!sadeque_push_back(&self->submitted_allocations , &allocation))
        {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }
    }
    vector_clear(&self->pending_frame_allocations);
}
