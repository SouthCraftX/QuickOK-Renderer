#pragma once
#define __QOR_VULKAN_MAKE_DEBUG_MESSAGE_SRC__

#include "../rendering_env.h"

void
vulkan_send_debug_message(
    VkDebugUtilsMessageSeverityFlagBitsEXT  message_severity ,
    VkDebugUtilsMessageTypeFlagsEXT         message_types ,
    qo_ccstring_t                           message
) {
    VkDebugUtilsMessengerCallbackDataEXT  callback_data = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT ,
        .pNext = NULL ,
        .flags = 0 ,
        .pMessageIdName  = "Engine-generated" ,
        .messageIdNumber = 0 ,
        .pMessage = message ,
        .queueLabelCount = 0 ,
        .pQueueLabels = NULL ,
        .cmdBufLabelCount = 0 ,
        .pCmdBufLabels = NULL ,
        .objectCount = 0 ,
        .pObjects = NULL
    };
    vkSubmitDebugUtilsMessageEXT(g_vk_global_context.instance ,
        message_severity , message_types , &callback_data);
}
