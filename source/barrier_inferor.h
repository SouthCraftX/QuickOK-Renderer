#pragma once
#include <cstdint>
#define __QOR_BARRIER_INFEROR_SRC__
#include "rendering_env.h"
#include <stdio.h>
// Capable for vulkan 1.2
// For vulkan 1.3+ which synchronization2 is available as a core feature,
// Use _NewAccessInfoNew
struct __LegacyBarrierAccessInfo
{
    VkAccessFlags         access_mark;
    VkPipelineStageFlags  stage_mark;
};
typedef struct __LegacyBarrierAccessInfo _LegacyBarrierAccessInfo;

// Though synchronization2 is served by its corresponding extension before
// vulkan 1.3, we don't use it here.
#if defined (VK_API_VERSION_1_3)
struct __NewAccessInfoNew
{
    VkAccessFlags2KHR         access_mark;
    VkPipelineStageFlags2KHR  stage_mark;
};
typedef struct __NewAccessInfoNew _NewAccessInfoNew;
#endif // VK_API_VERSION_1_3

void
__report_case_not_found(
    VkImageLayout  layout
) {
    char  message[64];
    sprintf(message , "Layout %d fall into default case." , layout);
    VkDebugUtilsMessengerCallbackDataEXT  callback_data = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT ,
        .pNext = NULL ,
        .flags = 0 ,
        .pMessageIdName  = "Engine-generated" ,
        .messageIdNumber = INT32_MAX ,
        .pMessage = message ,
        .queueLabelCount = 0 ,
        .pQueueLabels = NULL ,
        .cmdBufLabelCount = 0 ,
        .pCmdBufLabels = NULL ,
        .objectCount = 0 ,
        .pObjects = NULL
    };
    vkSubmitDebugUtilsMessageEXT(
        g_vk_global_context.instance ,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT , &callback_data
    );
}

_LegacyBarrierAccessInfo
infer_legacy_access_info(
    VkImageLayout  layout
) {
    switch (layout)
    {
        // --- Initial/Final States ---
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return { 0 , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return { VK_ACCESS_HOST_WRITE_BIT , VK_PIPELINE_STAGE_HOST_BIT };

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return { 0 , VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };

        // --- Transfer Operations ---
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return { VK_ACCESS_TRANSFER_READ_BIT ,
                     VK_PIPELINE_STAGE_TRANSFER_BIT };

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return { VK_ACCESS_TRANSFER_WRITE_BIT ,
                     VK_PIPELINE_STAGE_TRANSFER_BIT };

        // --- Color Attachments ---
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return { VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        // --- Depth/Stencil Attachments ---
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ,
                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT };

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT ,
                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT };

        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: // Vulkan 1.1+
            return { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ,
                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT };

        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: // Vulkan 1.1+
            return { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT ,
                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT };

        // --- Shader Resources ---
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return { VK_ACCESS_SHADER_READ_BIT ,
                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                     VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR };

        case VK_IMAGE_LAYOUT_GENERAL: // Used for storage images
            return { VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT ,
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };

        // --- Other Advanced Layouts ---
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR: // Vulkan 1.2+
            return { VK_ACCESS_SHADER_READ_BIT ,
                     VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT |
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };

        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR: // Vulkan 1.2+
            return { VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ,
                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        // We don't use vulkan directly to encode or decode videos
        // But we keep it here
        // case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        //     return { VK_ACCESS_VIDEO_DECODE_WRITE_BIT_KHR ,
        //              VK_PIPELINE_STAGE_VIDEO_DECODE_BIT_KHR };

        // case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        //     return { VK_ACCESS_VIDEO_DECODE_READ_BIT_KHR ,
        //              VK_PIPELINE_STAGE_VIDEO_DECODE_BIT_KHR };

        // case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        //     return { VK_ACCESS_VIDEO_ENCODE_READ_BIT_KHR ,
        //              VK_PIPELINE_STAGE_VIDEO_ENCODE_BIT_KHR };

        // case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        //     return { VK_ACCESS_VIDEO_ENCODE_WRITE_BIT_KHR ,
        //              VK_PIPELINE_STAGE_VIDEO_ENCODE_BIT_KHR };

        default:
            return { 0 , VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
    }
}

#if defined (VK_API_VERSION_1_3)
_NewAccessInfoNew
infer_new_access_info(
    VkImageLayout  layout
) {
    // TODO: Impl
    QO_ABORT();
}
#endif // VK_API_VERSION_1_3