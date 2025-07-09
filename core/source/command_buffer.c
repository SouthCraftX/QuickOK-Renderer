#include "command_buffer.h"
#include "container/vector.h"
#include "wrapped_vulkan_objects/wvkimage.h"
#include "access_info_inferor.h"
#include <vulkan/vulkan_core.h>

#define NO_MEMORY_RETURN(x)  if (!(x)) return VK_ERROR_OUT_OF_HOST_MEMORY

VkResult
command_buffer_pipeline_barrier(
    VkCommandBuffer                 command_buffer ,
    _GlobalMemoryDependency const * global_dependency ,
    _ImageTransition *              image_transitions ,
    _BufferTransition *             buffer_transitions ,
    qo_uint32_t                     image_transition_count ,
    qo_uint32_t                     buffer_transition_count
) {
    if (!image_transition_count &&
        !buffer_transition_count &&
        !global_dependency->source_access && !global_dependency->target_access)
    {
        return VK_SUCCESS;
    }

    VkPipelineStageFlags  source_stage_mask = 0;
    VkPipelineStageFlags  target_stage_mask = 0;

    // TODO: Free memory if return in advance
    _Vector  vkmemory_barriers;
    if (global_dependency->source_access || global_dependency->target_access)
    {
        NO_MEMORY_RETURN(
            vector_init(&vkmemory_barriers , sizeof(VkMemoryBarrier))
        );
    }

    _Vector  vkimage_barriers;
    NO_MEMORY_RETURN(
        vector_init(&vkimage_barriers , sizeof(VkImageMemoryBarrier))
    );
    NO_MEMORY_RETURN(
        vector_reserve(&vkimage_barriers , image_transition_count)
    );

    _Vector  vkbuffer_barriers;
    NO_MEMORY_RETURN(
        vector_init(&vkbuffer_barriers , sizeof(VkBufferMemoryBarrier))
    );

    if (global_dependency->source_access || global_dependency->target_access)
    {
        VkMemoryBarrier  memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER ,
            .pNext = NULL ,
            .srcAccessMask = global_dependency->source_access ,
            .dstAccessMask = global_dependency->target_access
        };
        NO_MEMORY_RETURN(
            vector_push_back(&vkmemory_barriers , &memory_barrier)
        );
    }

    for (qo_uint32_t i = 0; i < image_transition_count; ++i)
    {
        _ImageTransition * transition = image_transitions + i;
        
        _LegacyBarrierAccessInfo  source_info =
            infer_legacy_access_info(transition->old_layout);
        _LegacyBarrierAccessInfo  target_info =
            infer_legacy_access_info(transition->new_layout);

        source_stage_mask |= source_info.stage_mask;
        target_stage_mask |= target_info.stage_mask;

        VkImageMemoryBarrier  barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER ,
            .pNext = NULL ,
            .srcAccessMask = source_info.access_mask ,
            .dstAccessMask = target_info.access_mask ,
            .oldLayout = transition->old_layout ,
            .newLayout = transition->new_layout ,
            .srcQueueFamilyIndex = transition->source_queue_family ,
            .dstQueueFamilyIndex = transition->target_queue_family ,
            .image = wvkimage_get_handle(transition->image)
        };

        if (transition->subresource_range.has_value)
        {
            barrier.subresourceRange = transition->subresource_range.data;
        }
        else
        {
            VkFormat  format = wvkimage_get_format(transition->image);
            if (format >= VK_FORMAT_D16_UNORM &&
                format <= VK_FORMAT_D32_SFLOAT_S8_UINT)
            {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                    format == VK_FORMAT_D24_UNORM_S8_UINT)
                {
                    barrier.subresourceRange.aspectMask |=
                        VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else
            {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

            NO_MEMORY_RETURN(vector_push_back(&vkimage_barriers , &barrier));
        }
    }

    for (qo_uint32_t i = 0; i < buffer_transition_count; ++i)
    {
        _BufferTransition * transition = buffer_transitions + i;

        VkBufferMemoryBarrier  barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER ,
            .pNext = NULL ,
            .srcAccessMask = transition->source_access ,
            .dstAccessMask = transition->target_access ,
            .srcQueueFamilyIndex = transition->source_queue_family ,
            .dstQueueFamilyIndex = transition->target_queue_family ,
            .buffer = wvkbuffer_get_handle(transition->buffer)
        };

        if (transition->use_offset_and_size)
        {
            barrier.offset = transition->offset;
            barrier.size = transition->size;
        }
        else 
        {
            barrier.offset = 0;
            barrier.size = VK_WHOLE_SIZE;
        }


        NO_MEMORY_RETURN(vector_push_back(&vkbuffer_barriers , &barrier));
    }

    if (!source_stage_mask)
    {
        source_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (!target_stage_mask)
    {
        target_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(
        command_buffer , source_stage_mask , target_stage_mask , 0 , 0 , NULL ,
        0 , NULL , 0 , NULL
    );
}
