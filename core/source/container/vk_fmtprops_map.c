#include "vk_fmtprops_map.h"
#include "funnel_hash_table.h"
#include <mimalloc.h>
#include <vulkan/vulkan_core.h>

qo_uint64_t
hash_vk_format(
    fht_key_t   key ,
    qo_uint32_t salt
) {
    qo_uint64_t h = key;
    h ^= salt;
    h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
    h = h ^ (h >> 31);
    return h;
}

qo_bool_t
vk_format_equals(
    fht_key_t   a ,
    fht_key_t   b
) {
    return a == b;
}

qo_stat_t
vk_fmtprops_map_init(
    _VkFormatPropertiesMap *    self, 
    qo_size_t                   capicity
) {
    return fht_init(
        &self->hash_table,
        capicity,
        .1f , // TODO: Use macro
        &(_FunnelHashTableAuxiliary){
            .hash_func = hash_vk_format,
            .equals_func = vk_format_equals ,
            .key_destroy_func = NULL
        }
    );
}

void
vk_fmtprops_map_destroy(
    _VkFormatPropertiesMap *    self
) {
    _FunnelHashTableIterator iterator = fht_iterate(&self->hash_table);
    fht_key_t key;
    fht_value_t value;
    while(fht_iterator_next(&iterator, &key, &value))
    {
        mi_free((qo_pointer_t)value);
    }
    fht_destroy(&self->hash_table);
}

qo_bool_t
vk_fmtprops_map_insert(
    _VkFormatPropertiesMap *    self ,
    VkFormat                    format ,
    VkFormatProperties const *  properties
) {
    VkFormatProperties * copy = mi_malloc_tp(VkFormatProperties);
    if (!copy)
        return qo_false;
    
    memcpy(copy , properties , sizeof(VkFormatProperties));
    fht_key_t key = format;
    fht_value_t value = (fht_value_t)copy;
    if (fht_insert(&self->hash_table, key, value))
    { // Success
        return qo_true;
    }
    mi_free(copy);
    return qo_false;
}

VkFormatProperties const *
vk_fmtprops_map_search(
    _VkFormatPropertiesMap *    self ,
    VkFormat                    format
) {
    VkFormatProperties const * target;
    return fht_search(&self->hash_table , format , (fht_value_t *)&target) ?
            (VkFormatProperties const *)target : NULL;
}