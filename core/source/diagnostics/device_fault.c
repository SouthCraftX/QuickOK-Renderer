#include "rendering_env.h"
#include <stdio.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

QO_GLOBAL_LOCAL
void
send_message(
    PFN_vkDebugUtilsMessengerCallbackEXT callback ,
    VkDebugUtilsMessengerCallbackDataEXT* callback_data
) {
    callback(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, callback_data, NULL);
}

void
post_device_fault(
    PFN_vkDebugUtilsMessengerCallbackEXT callback ,
    VkDevice device
) {
    VkDeviceFaultCountsEXT  fault_count;
    VkDeviceFaultInfoEXT    fault_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT
    };
    VkDebugUtilsMessengerCallbackDataEXT callback_data = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT ,
        .pNext = NULL ,
        .messageIdNumber = 0,
        .pMessage = "[Device Fault Report] BEGIN",
        .pMessageIdName = "Engine-generated" ,
        .pCmdBufLabels = NULL ,
        .cmdBufLabelCount = 0 ,
        .flags = 0,
        .pObjects = NULL ,
        .objectCount = 0
    };
    
    VkResult res = vkGetDeviceFaultInfoEXT(device, &fault_count, &fault_info);
    callback(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &callback_data , NULL);

    char message[VK_MAX_DESCRIPTION_SIZE + 128];
    if (res == VK_SUCCESS)
    {
        sprintf(message, "[Device Fault Report] VkDeviceFaultInfoEXT.description: %s", fault_info.description);
        callback_data.pMessage = message;
        send_message(callback, &callback_data);
        sprintf(message, "[Device Fault Report] addressInfoCount: %d", fault_count.addressInfoCount);
        send_message(callback, &callback_data);
        for (uint32_t i = 0; i < fault_count.addressInfoCount; i++)
        {
            sprintf(message, "[Device Fault Report] addressInfo[%d].AddressType: %d", i, fault_info.pAddressInfos[i].addressType);
            send_message(callback, &callback_data);
            sprintf(message, "[Device Fault Report] addressInfo[%d].AddressPrecision: %llu", i, fault_info.pAddressInfos[i].addressPrecision);
            send_message(callback, &callback_data);
            sprintf(message, "[Device Fault Report] addressInfo[%d].ReportedAddress: %llu", i, fault_info.pAddressInfos[i].reportedAddress);
            send_message(callback, &callback_data);
        }

        for (uint32_t i = 0 ; i < fault_count.vendorInfoCount ; i++)
        {
            sprintf(message, "[Device Fault Report] vendorInfo[%d].vendorFaultCode: %llu", i, fault_info.pVendorInfos[i].vendorFaultCode);
            send_message(callback, &callback_data);
            sprintf(message, "[Device Fault Report] vendorInfo[%d].vendorFaultData: %llu", i, fault_info.pVendorInfos[i].vendorFaultData);
            send_message(callback, &callback_data);
            sprintf(message, "[Device Fault Report] vendorInfo[%d].vendorFaultDataSize: %llu", i, fault_info.pVendorInfos[i].vendorFaultDataSize);
            send_message(callback, &callback_data);
        }

        sprintf(message, "[Device Fault Report] pVendorBinaryData: 0x%p", fault_info.pVendorBinaryData);
        send_message(callback, &callback_data);
        sprintf(message, "[Device Fault Report] vendorBinarySize: %llu", fault_count.vendorBinarySize);
        send_message(callback, &callback_data);
    }
    else {
        sprintf(message, "[Device Fault Report] vkGetDeviceFaultInfoEXT failed with %d", res);
        callback_data.pMessage = message;
        send_message(callback, &callback_data);
    }
    callback_data.pMessage = "[Device Fault Report] END";
    send_message(callback, &callback_data);
}

