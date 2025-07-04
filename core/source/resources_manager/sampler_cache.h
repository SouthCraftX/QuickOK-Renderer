#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_SAMPLER_CACHE_SRC__

#include "../container/sampler_map.h"

struct __SamplerCache
{
    _SamplerMap sampler_map;
    _VkDeviceContext *  device_context;
};
typedef struct __SamplerCache _SamplerCache;

QO_NODISCARD
QO_NONNULL(1)
qo_stat_t
sampler_cache_init(
    _SamplerCache * self,
    _VkDeviceContext * device_context
);

void
sampler_cache_destroy(
    _SamplerCache * self
);

QO_NODISCARD QO_NONNULL(1 , 2)
VkSampler
sampler_cache_get(
    _SamplerCache * self ,
    VkSamplerCreateInfo const * sampler_info
);

/* Common Samplers */
QO_NODISCARD QO_NONNULL(1)
VkSampler
get_linear_repeat_sampler(
    _SamplerCache * self
);

QO_NODISCARD QO_NONNULL(1)
VkSampler
get_linear_clamp_sampler(
    _SamplerCache * self
);

QO_NODISCARD QO_NONNULL(1)
VkSampler
get_nearest_clamp_sampler(
    _SamplerCache * self
);


