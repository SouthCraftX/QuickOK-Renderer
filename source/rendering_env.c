#include "rendering_env.h"
#include "mimalloc_vulkan_callback.h"
#include <vulkan/vulkan_core.h>

// Destroy everything except VkInstance
qo_stat_t
qo_rendering_env_hibernate(
    _VkContext * context
) {
}

qo_stat_t
qo_rendering_env_resume(
    _VkContext * context
) {
}

qo_stat_t
create_vkinstance(
    _VkContext * context ,
    qo_flag32_t           flags
) {
    VkApplicationInfo  app_info = {
        .pApplicationName = "QuickOK Renderer" ,
        .applicationVersion = VK_MAKE_VERSION(1 , 0 , 0) ,
        .pEngineName = "No Engine" ,
        .engineVersion = VK_MAKE_VERSION(1 , 0 , 0) ,
        .apiVersion = VK_API_VERSION_1_2 ,   // TODO: make this a parameter
        .pNext = NULL
    };

    VkInstanceCreateInfo  instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO ,
        .pApplicationInfo = &app_info ,
        .pNext = NULL ,
        .flags = 0 ,
        .enabledExtensionCount = 0 ,
        .enabledLayerCount = 0
    };

    qo_ccstring_t * glfw_extensions = NULL;
    uint32_t  glfw_extension_count  = 0;

    if (flags & INSTANCE_CREATE_WITH_VALIDATOR)
    {
        instance_create_info.enabledLayerCount += 1;
    }
    if (flags & INSTANCE_CREATE_WITH_GLFW)
    {
        glfw_extensions =
            glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        instance_create_info.enabledExtensionCount += glfw_extension_count;
    }

    SAFEGUARD_VLA_ELEMENT_COUNT_COUNT(
        instance_create_info.enabledExtensionCount
    );
    qo_ccstring_t  extensions[instance_create_info.enabledExtensionCount];

    // We don't want to slow down creation, so we don't check extensions availability
    // here. We'll check it when creation fails
    // TODO: enable validation layer if requested
    memcpy(extensions , glfw_extensions ,
        instance_create_info.enabledExtensionCount * sizeof(qo_ccstring_t));
    if (flags & INSTANCE_CREATE_WITH_VALIDATOR)
    {
        extensions[instance_create_info.enabledExtensionCount -
                   1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    instance_create_info.ppEnabledExtensionNames = extensions;
    context->desired_extensions = extensions;
    context->desired_extension_count =
        instance_create_info.enabledExtensionCount;

    // Slowest call. Expect 2000 ms +
    vkCreateInstance(&instance_create_info , &g_vk_mimallocator , &context->instance);

}

qo_int32_t
make_supported_layers_available(
    _VkContext * context
) {
    SUPRESS_PTR_SIGN_WARNING_BEGIN; // int <-> uint
    qo_int32_t  layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count , NULL);
    if (layer_count)
    {
        VkLayerProperties * layers = mi_mallocn_tp(VkLayerProperties ,
            layer_count);
        if (!layers)
        {
            return -1;
        }
        vkEnumerateInstanceLayerProperties(&layer_count , layers);
        context->supported_layers = layers;
        context->supported_layer_count = layer_count;
    }
    return 0;
    SUPRESS_PTR_SIGN_WARNING_END;
}

qo_int32_t
make_supported_extensions_available(
    _VkContext * context
) {
    switch (context->supported_layer_count)
    {
        case -1:
            make_supported_layers_available(context);
            break;

        case 0:
            context->supported_extension_count = 0;
            context->supported_extensions = NULL;
            return 0;

        default:
            break;
    }

    qo_uint32_t  extension_count = 0;
    // We need to query each layer one by one because setting pLayerName to NULL
    // and querying only get extensions of the core layer
    for (qo_uint32_t i = 0; i < context->supported_layer_count; i++)
    {
        vkEnumerateInstanceExtensionProperties(
            context->supported_layers[i].layerName , &extension_count , NULL
        );
    }

    if (extension_count)
    {
        VkExtensionProperties * extensions =
            mi_mallocn_tp(VkExtensionProperties , extension_count);
        if (!extensions)
        {
            return -1;
        }

        qo_uint32_t  this_count = extension_count;
        VkExtensionProperties * write_pos = extensions;
        // Query core extensions first
        vkEnumerateInstanceExtensionProperties(NULL , &this_count , write_pos);
        write_pos += this_count;

        if (extension_count - this_count) // Have other extensions in other layers
        {
            for (qo_uint32_t i = 0; i < context->supported_layer_count; i++)
            {
                vkEnumerateInstanceExtensionProperties(
                    context->supported_layers[i].layerName , &this_count ,
                    write_pos
                );
                write_pos += this_count;
            }
        }

        context->supported_extensions = extensions;
        context->supported_extension_count = extension_count;
    }
    return 0;
}

qo_int32_t
get_supported_extensions(
    _VkContext *    context ,
    VkExtensionProperties ** p_extensions
) {
L_BEGIN:
    switch (context->supported_extension_count)
    {
        case -1:
            make_supported_extensions_available(context);
            goto L_BEGIN;

        case 0:
            *p_extensions = NULL;
            return 0;

        default:
            *p_extensions = context->supported_extensions;
            return context->supported_extension_count;
    }
}

qo_int32_t
get_supported_layers(
    _VkContext * context ,
    VkLayerProperties **  p_layers
) {
L_BEGIN:
    switch (context->supported_layer_count)
    {
        case -1:
            make_supported_layers_available(context);
            goto L_BEGIN;

        case 0:
            *p_layers = NULL;
            return 0;

        default:
            *p_layers = context->supported_layers;
            return context->supported_layer_count;
    }
}

// Return immediately if a unavailable extension is found
// NULL will be returned if all extensions are available
VkExtensionProperties *
check_extensions_availability(
    _VkContext * context ,
    qo_int32_t            index
) {
    VkExtensionProperties * supported_extensions;
    qo_int32_t  supported_count = get_supported_extensions(context ,
        &supported_extensions);

    if (supported_count == 0)
    {
        return NULL;  // No extensions available
    }
    qo_bool_t  found;
    VkExtensionProperties * desired = context->desired_extensions + index;
    for (;
         desired <
         context->desired_extensions + context->desired_extension_count ;
         ++desired)
    {
        found = false;
        for (VkExtensionProperties * supported = supported_extensions ;
             supported < supported_extensions + supported_count ;
             ++supported)
        {
            if (!strcmp(desired->extensionName , supported->extensionName))
            {
                found = true;
                break; // back to `desired`-level loop
            }
        }
        if (!found)
        {
            return desired;
        }
    }
    return NULL;
}

VkLayerProperties *
check_layers_availability(
    _VkContext * context ,
    qo_int32_t            index
) {
    VkLayerProperties * supported_layers;
    qo_int32_t  supported_count = get_supported_layers(context ,
        &supported_layers);
    if (!supported_count)
    {
        return NULL;
    }

    qo_bool_t  found;
    VkLayerProperties * desired = context->desired_layers + index;
    for (; desired < context->desired_layers + context->desired_layer_count ;
         ++desired)
    {
        found = false;
        for (VkLayerProperties * supported = supported_layers ;
             supported < supported_layers + supported_count ;
             ++supported)
        {
            if (!strcmp(desired->layerName , supported->layerName))
            {
                found = true;
                break; // back to `desired`-level loop
            }
        }
        if (!found)
        {
            return desired;
        }
    }
    return NULL;
}

qo_stat_t
qo_get_available_gpus(
    _VkContext * context ,
    QO_GPUPicker *        p_picker
) {
    qo_uint32_t  device_count = 0;
    vkEnumeratePhysicalDevices(context->instance , &device_count , NULL);
    if (device_count)
    {
        vkEnumeratePhysicalDevices(context->instance , &device_count ,
            p_picker->devices);
    }
    p_picker->count = device_count;
}

VkPhysicalDevice
qo_find_first_matched_gpu(
    QO_GPUPicker *           picker ,
    VkQueueFamilyProperties  queue_family_properties ,
    qo_int8_t                index ,
    qo_uint32_t *            p_queue_family_index
) {
    VkPhysicalDevice  matched = NULL;
    for (; index < picker->count; index++)
    {
        qo_uint32_t  queue_family_count = 0;
        VkPhysicalDevice  device = picker->devices[index];
        vkGetPhysicalDeviceQueueFamilyProperties(device , &queue_family_count ,
            NULL);
        SAFEGUARD_VLA_ELEMENT_COUNT_COUNT(queue_family_count);
        VkQueueFamilyProperties  queue_families[queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(device , &queue_family_count ,
            queue_families);

        qo_int32_t  queue_family_index = -1;
        for (qo_uint32_t i = 0; i < queue_family_count; i++)
        {
            if (queue_families[i].queueFlags &
                queue_family_properties.queueFlags)
            {
                queue_family_index = i;
                break;
            }
        }
        if (queue_family_index != -1)
        {
            matched = device;
            *p_queue_family_index = queue_family_index;
            break;
        }
    }
    return matched;
}

#define QUEUE_CREATE_INFO(x)  { \
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO , \
            .queueFamilyIndex = p_indexes->x ## _families[0] , \
            .queueCount = 1 , \
            .pQueuePriorities = &queue_priority , \
            .flags = 0 \
}

void
create_logical_deivce(
    _VkContext *   context ,
    _VkQueueFamilyIndexes * p_indexes ,
    _VkQueueFamilyCounts *  capabilities
) {
    qo_fp32_t  queue_priority = 1.;
    VkDeviceQueueCreateInfo  queue_create_infos[] = {
        QUEUE_CREATE_INFO(graphics) ,
        QUEUE_CREATE_INFO(compute) ,
        QUEUE_CREATE_INFO(transfer)
    };

    VkPhysicalDeviceFeatures  device_features;
    VkDeviceCreateInfo  device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO ,
        .pNext = NULL ,
        .flags = 0 ,
        .queueCreateInfoCount = 3 ,
        .pQueueCreateInfos = queue_create_infos ,
        .enabledExtensionCount = 0 ,
        .ppEnabledExtensionNames = NULL ,
        .pEnabledFeatures = &device_features ,
    };

    // TODO: Error handling
    vkCreateDevice(context->physical_device , &device_create_info , &g_vk_mimallocator ,
        &context->logical_device);
    vkGetDeviceQueue(context->logical_device , p_indexes->graphics_families[0] ,
        0 , &context->queues.graphics);
    vkGetDeviceQueue(context->logical_device , p_indexes->compute_families[0] ,
        0 , &context->queues.compute);
    vkGetDeviceQueue(context->logical_device , p_indexes->transfer_families[0] ,
        0 , &context->queues.transfer);

    context->queue_family_indexes.graphics = p_indexes->graphics_families[0];
    context->queue_family_indexes.compute  = p_indexes->compute_families[0];
    context->queue_family_indexes.transfer = p_indexes->transfer_families[0];
}

typedef enum
{
    VK_CMD_TYPE_GRAPHICS = 0 ,
    VK_CMD_TYPE_COMPUTE = 1 ,
    VK_CMD_TYPE_TRANSFER = 2 ,
} _VkCmdType;

qo_stat_t
qo_alloc_thread_cmd(
    _VkContext * context ,
    _VkThreadLocalCmd *   cmd ,
    _VkCmdType            type
) {
    VkCommandPoolCreateInfo  cmd_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO ,
        .pNext = NULL ,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ,
        .queueFamilyIndex = context->queue_family_indexes.qfi3[type] ,
    };
    // TODO: Error handling
    vkCreateCommandPool(context->logical_device , &cmd_pool_create_info , &g_vk_mimallocator ,
        &cmd->command_pools.cp3[type]);
}

qo_stat_t
qo_free_thread_cmd(
    _VkContext * context ,
    _VkThreadLocalCmd *   cmd ,
    _VkCmdType            type
) {
    vkDestroyCommandPool(context->logical_device ,
        cmd->command_pools.cp3[type] , &g_vk_mimallocator);

}

#if defined (DEBUG)

VkBool32
vk_debug_messager(
    VkDebugUtilsMessageSeverityFlagBitsEXT       message_severity ,
    VkDebugUtilsMessageTypeFlagsEXT              message_type ,
    VkDebugUtilsMessengerCallbackDataEXT const * p_callback_data ,
    void *                                       p_user_data
) {
    printf("validation layer: %s\n" , p_callback_data->pMessage);
    return VK_FALSE;
}

void
create_debug_messager(
    _VkContext *                 context ,
    PFN_vkDebugUtilsMessengerCallbackEXT  callback
) {
    VkDebugUtilsMessengerCreateInfoEXT  debug_messenger_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT ,
        .pNext = NULL ,
        .flags = 0 ,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ,
        .pfnUserCallback = callback ,
    };
    // TODO: Error handling
    // The only possible error: VK_ERROR_OUT_OF_HOST_MEMORY
    VkResult  result = vkCreateDebugUtilsMessengerEXT(
        context->instance , &debug_messenger_create_info , &g_vk_mimallocator ,
        &context->debug_messenger
    );
}

void
destroy_debug_messenger(
    _VkContext * context
) {
    vkDestroyDebugUtilsMessengerEXT(context->instance ,
        context->debug_messenger , &g_vk_mimallocator);
}

#endif
