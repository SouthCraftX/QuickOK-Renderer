#pragma once

#include "barrier_builder.h"
#include "container/vector.h"
#include "container/optional.h"
#include "rendering_env.h"
#include "wrapped_vulkan_objects/wvkbuffer.h"
#include "wrapped_vulkan_objects/wvkimage.h"

struct __ImageTransition
{
    _WVkImage *    image;
    VkImageLayout  old_layout;
    VkImageLayout  new_layout;
    _OPTIONAL(VkImageSubresourceRange , subresource_range);
    qo_uint32_t    source_queue_family;
    qo_uint32_t    target_queue_family;
};
typedef struct __ImageTransition _ImageTransition;

struct __BufferTransition
{
    _WVkBuffer *          buffer;
    VkPipelineStageFlags  source_stage;
    VkAccessFlags         source_access;
    VkPipelineStageFlags  target_stage;
    VkAccessFlags         target_access;
    qo_uint32_t           source_queue_family;
    qo_uint32_t           target_queue_family;

    qo_bool_t             use_offset_and_size;
    VkDeviceSize          offset;
    VkDeviceSize          size;
};
typedef struct __BufferTransition _BufferTransition;

struct __GlobalMemoryDependency
{
    VkPipelineStageFlags  source_stage;
    VkAccessFlags         source_access;
    VkPipelineStageFlags  target_stage;
    VkAccessFlags         target_access;
};
typedef struct __GlobalMemoryDependency _GlobalMemoryDependency;

QO_NODISCARD QO_NONNULL(1 , 2) // TODO: Rethink nonnull
VkResult
command_buffer_pipeline_barrier(
    VkCommandBuffer                 command_buffer ,
    _GlobalMemoryDependency const * global_dependency ,
    _ImageTransition *              image_transitions ,
    _BufferTransition *             buffer_transitions ,
    qo_uint32_t                     image_transition_count ,
    qo_uint32_t                     buffer_transition_count
);