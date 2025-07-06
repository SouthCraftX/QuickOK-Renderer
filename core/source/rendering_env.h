#pragma once
#include "mimalloc_vulkan_callback.h"
#include "../include/renderer.h"
#include "container/vk_fmtprops_map.h"
#include <mimalloc.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

typedef qo_uint64_t object_id_t;
typedef qo_pointer_t object_ptr_t;
typedef qo_uint64_t   state_id_t;

struct __VkQueueFamilyIndexes
{
    // Some GPUs provide mutilple
    qo_int8_t  graphics_families[4];
    qo_int8_t  compute_families[4];
    qo_int8_t  transfer_families[4];
};
typedef struct __VkQueueFamilyIndexes _VkQueueFamilyIndexes;

struct __VkQueueFamilyCounts
{
    qo_uint8_t  graphics_family_count: 4;
    qo_uint8_t  compute_family_count : 4;
    qo_uint8_t  transfer_family_count: 4;
};
typedef struct __VkQueueFamilyCounts _VkQueueFamilyCounts;

// Screen or Offscreen
struct __FinalTarget;
typedef struct __FinalTarget _FinalTarget;

// We separate the global context from the device context.
// Reserve for future multi-device support.
struct __VkGlobalContext
{
    VkInstance                  instance;
    // We enable it even in release to deliver vulkan-level error
    VkDebugUtilsMessengerEXT    debug_messenger;
    VmaAllocator                vma_allocator;
    VkAllocationCallbacks       mi_malloc_callbacks;
};
typedef struct __VkGlobalContext _VkGlobalContext;
_VkGlobalContext g_vk_global_context;

QO_FORCE_INLINE
VkAllocationCallbacks *
get_vk_allocator()
{
    return &g_vk_global_context.mi_malloc_callbacks;
}

QO_FORCE_INLINE
VmaAllocator 
get_vma_allocator()
{
    return g_vk_global_context.vma_allocator;
}


// Binding to a specific device
// One device per context
struct __VkDeviceContext
{
    VkResult  internal_result;

    /* Layers and extensions */
    // Remark: if *_count is -1, it means this infomation is not ready
    // if it is 0, it means that there is no extension or layer
    VkExtensionProperties * supported_extensions;
    VkLayerProperties *     supported_layers;
    VkExtensionProperties * desired_extensions;
    VkLayerProperties *     desired_layers;
    qo_uint32_t             supported_extension_count;
    qo_uint32_t             supported_layer_count;
    uint32_t                desired_extension_count;
    uint32_t                desired_layer_count;

    // Remark: Given that VkInstance took loog to create, we may have to keep it
    // instead of destroying it.
    VkPhysicalDevice                  physical_device;
    VkDevice                          logical_device;
    VkPhysicalDeviceProperties        physical_device_properties;
    VkPhysicalDeviceMemoryProperties  physical_device_memory_properties;

    // We just need one queue for each family, no matter how complicated the
    // rendering is.
    union
    {
        struct
        {
            VkQueue  graphics;
            VkQueue  compute;
            VkQueue  transfer;
        };
        VkQueue  q3[3];
    } queues;

    union
    {
        struct
        {
            qo_uint8_t  graphics;
            qo_uint8_t  compute;
            qo_uint8_t  transfer;
        };
        qo_uint8_t  qfi3[3];
    } queue_family_indexes;

    VkPipeline                pipeline;
    VkPipelineLayout          pipeline_layout;

    struct 
    {
        VkMemoryRequirements image;
        VkMemoryRequirements buffer;
    } memory_requirements;

    _VkFormatPropertiesMap    format_properties_map;
};
typedef struct __VkDeviceContext _VkDeviceContext;

struct __VkThreadLocalCmd
{
    union
    {
        struct
        {
            VkCommandBuffer  graphics;
            VkCommandBuffer  compute;
            VkCommandBuffer  transfer;
        };
        VkCommandBuffer  cb3[3];
    } command_buffers;
    union
    {
        struct
        {
            VkCommandPool  graphics;
            VkCommandPool  compute;
            VkCommandPool  transfer;
        };
        VkCommandPool  cp3[3];
    } command_pools;
};
typedef struct __VkThreadLocalCmd _VkThreadLocalCmd;

struct __VkOffscreenContext
{
    VkImage         image;
    VkDeviceMemory  image_memory;
    VkImageView     image_view;
    VkRenderPass    render_pass;
    VkFramebuffer   frame_buffer;
};
typedef struct __VkOffscreenContext _VkOffscreenContext;

struct __VkShader
{
    VkShaderModule   shader_module;
    VkStructureType  stage;
    qo_ccstring_t    entry_func_name;
};

#define INSTANCE_CREATE_WITH_GLFW       1
#define INSTANCE_CREATE_WITH_VALIDATOR  (1 << 1)

#define VLA_MAX_ELEMENT_COUNT           256
#define SAFEGUARD_VLA_ELEMENT_COUNT_COUNT(x)  if (x >= \
                                                  VLA_MAX_ELEMENT_COUNT)(x) = \
    VLA_MAX_ELEMENT_COUNT - 1

#if defined (__clang__)
    # define SUPRESS_PTR_SIGN_WARNING_BEGIN \
            _Pragma("clang diagnostic push") \
            _Pragma("clang diagnostic ignored \"-Wpointer-sign\"")
    # define SUPRESS_PTR_SIGN_WARNING_END \
            _Pragma("clang diagnostic pop")
#elif defined (__GNUC__)
    # define SUPRESS_PTR_SIGN_WARNING_BEGIN \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wpointer-sign\"")
    # define SUPRESS_PTR_SIGN_WARNING_END \
            _Pragma("GCC diagnostic pop")
#elif defined (_MSC_VER)
    # define SUPRESS_PTR_SIGN_WARNING_BEGIN \
            __pragma(warning(push)) \
            __pragma(warning(disable: 4028)) \// TODO: Check them
__pragma(warning(disable: 4133))
    # define SUPRESS_PTR_SIGN_WARNING_END \
            __pragma(warning(pop))
#endif
