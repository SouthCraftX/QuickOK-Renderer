#pragma once
#define __QOR_DESCRIPTOR_SET_LAYOUT_BINDING_MAP_SRC__

#include <mimalloc.h>
#include "funnel_hash_table.h"

struct __DescriptorSetLayoutBindingMap
{
    _FunnelHashTable  hash_table;
    mi_heap_t *       value_heap; // Make it easy to free objects
};
typedef struct __DescriptorSetLayoutBindingMap _DescriptorSetLayoutBindingMap;

QO_NONNULL(1)
qo_stat_t
descriptor_set_layout_binding_map_init(
    _DescriptorSetLayoutBindingMap * self
);

void
descriptor_set_layout_binding_map_destory(
    _DescriptorSetLayoutBindingMap * self
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
descriptor_set_layout_binding_map_set(
    _DescriptorSetLayoutBindingMap *     self ,
    qo_uint32_t                          key ,
    VkDescriptorSetLayoutBinding const * value
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
descriptor_set_layout_binding_map_search(
    _DescriptorSetLayoutBindingMap * self ,
    qo_uint32_t                      key ,
    VkDescriptorSetLayoutBinding *   p_value
);
