#include "descriptor_set_layout_binding_map.h"
#include "funnel_hash_table.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>
#include <xxh3.h>

QO_GLOBAL_LOCAL
qo_bool_t
VkDescriptorSetLayout_compare(
    fht_value_t  x ,
    fht_value_t  y
) {
    VkDescriptorSetLayoutBinding const * const  x_layout =
        (VkDescriptorSetLayoutBinding const *) x;
    VkDescriptorSetLayoutBinding const * const  y_layout =
        (VkDescriptorSetLayoutBinding const *) y;
    return
        x_layout->binding == y_layout->binding &&
        x_layout->descriptorCount == y_layout->descriptorCount &&
        x_layout->stageFlags == y_layout->stageFlags &&
        x_layout->pImmutableSamplers == y_layout->pImmutableSamplers &&
        x_layout->descriptorType == y_layout->descriptorType
    ;
}

QO_GLOBAL_LOCAL
qo_bool_t
key_compare(
    fht_key_t  x ,
    fht_key_t  y
) {
    return x == y;
}

QO_GLOBAL_LOCAL
XXH64_hash_t
key_direct_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    return key ^ salt;
}

qo_stat_t
descriptor_set_layout_binding_map_init(
    _DescriptorSetLayoutBindingMap * self
) {
    self->value_heap = mi_heap_new();
    if (!self->value_heap)
    {
        return QO_OUT_OF_MEMORY;
    }
    qo_stat_t  ret = fht_init(
        &self->hash_table , 0 , .1 , &(_FunnelHashTableAuxiliary) {
        .hash_func = key_direct_hash ,
        .equals_func = key_compare ,
        .key_destroy_func = NULL
    }
    );
    if (ret)
    {
        mi_heap_delete(self->value_heap);
    }
    return ret;
}

void
descriptor_set_layout_binding_map_destroy(
    _DescriptorSetLayoutBindingMap * self
) {
    mi_heap_delete(self->value_heap);
    fht_destroy(&self->hash_table);
}

qo_bool_t
descriptor_set_layout_binding_map_set(
    _DescriptorSetLayoutBindingMap *     self ,
    qo_uint32_t                          key ,
    VkDescriptorSetLayoutBinding const * value
) {
    _FunnelHashTableEntry * entry = fht_find_slot(
        &self->hash_table , key , qo_true
    );
    if (!entry)
    {
        return qo_false;
    }

    if (entry->value != FHT_EMPTY_VALUE) // Update Path
    {
        VkDescriptorSetLayoutBinding * existing_ptr =
            (VkDescriptorSetLayoutBinding *) entry->value;
        memcpy(existing_ptr , value , sizeof(VkDescriptorSetLayoutBinding));
    }
    else // Insert Path
    {
        if (fht_get_count(&self->hash_table) >= self->hash_table.capacity)
        {
            return qo_false;
        }
        VkDescriptorSetLayoutBinding * copy =
            mi_heap_malloc_tp(self->value_heap , VkDescriptorSetLayoutBinding);
        memcpy(copy , value , sizeof(VkDescriptorSetLayoutBinding));
        entry->value = (fht_value_t) copy;
        entry->key = key;
        self->hash_table.num_inserts++;
    }
    return qo_true;
}

qo_bool_t
descriptor_set_layout_binding_map_search(
    _DescriptorSetLayoutBindingMap * self ,
    qo_uint32_t                      key ,
    VkDescriptorSetLayoutBinding *   p_value
) {
    return fht_search(
        &self->hash_table , key , (fht_value_t *) p_value
    );
}
