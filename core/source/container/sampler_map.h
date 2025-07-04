#pragma once
#define __QOR_SAMPLER_MAP_H__

#include "funnel_hash_table.h"

struct __SamplerMap
{
    _FunnelHashTable hash_table;
};
typedef struct __SamplerMap _SamplerMap;

QO_NODISCARD
qo_stat_t
sampler_map_init(
    _SamplerMap * self
);

void
sampler_map_destroy(
    _SamplerMap * self
);

QO_NODISCARD
qo_bool_t
sampler_map_insert(
    _SamplerMap *   self ,
    VkSamplerCreateInfo const * key ,
    qo_uint64_t                 value
);

QO_NODISCARD
qo_bool_t
sampler_map_set(
    _SamplerMap * self ,
    VkSamplerCreateInfo const * key ,
    qo_uint64_t                 value
);

QO_NODISCARD
qo_bool_t
sampler_map_search(
    _SamplerMap * self ,
    VkSamplerCreateInfo const * key ,
    qo_uint64_t *               value
);
