#pragma once
#define __QOR_MIMALLOC_VULKAN_CALLBACK_SRC__

#include "rendering_env.h"
#include <vulkan/vulkan.h>

extern const VkAllocationCallbacks  g_vk_mimallocator;

void
vulkan_mimalloc_init();

void
vulkan_mimalloc_destroy();
