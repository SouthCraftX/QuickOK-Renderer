#include "sampler_map.h"
#include "funnel_hash_table.h"
#include <mimalloc.h>
#include <xxh3.h>

qo_bool_t
VkSamplerCreateInfo_compare(
    fht_key_t  x ,
    fht_key_t  y
) {
    VkSamplerCreateInfo const * const  s1 = (VkSamplerCreateInfo const *) x;
    VkSamplerCreateInfo const * const  s2 = (VkSamplerCreateInfo const *) y;

    return
        s1->flags == s2->flags && s1->magFilter == s2->magFilter &&
        s1->minFilter == s2->minFilter && s1->mipmapMode == s2->mipmapMode &&
        s1->addressModeU == s2->addressModeU &&
        s1->addressModeV == s2->addressModeV &&
        s1->addressModeW == s2->addressModeW &&
        s1->mipLodBias == s2->mipLodBias &&
        s1->anisotropyEnable == s2->anisotropyEnable &&
        s1->maxAnisotropy == s2->maxAnisotropy &&
        s1->compareEnable == s2->compareEnable &&
        s1->compareOp == s2->compareOp && s1->minLod == s2->minLod &&
        s1->maxLod == s2->maxLod && s1->borderColor == s2->borderColor &&
        s1->unnormalizedCoordinates == s2->unnormalizedCoordinates
    ;
}

XXH64_hash_t
VkSamplerCreateInfo_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    VkSamplerCreateInfo const * const  s = (VkSamplerCreateInfo const *) key;
    XXH3_state_t * const  state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);
    XXH3_64bits_update(state , &s->flags , sizeof(s->flags));
    XXH3_64bits_update(state , &s->magFilter , sizeof(s->magFilter));
    XXH3_64bits_update(state , &s->minFilter , sizeof(s->minFilter));
    XXH3_64bits_update(state , &s->mipmapMode , sizeof(s->mipmapMode));
    XXH3_64bits_update(state , &s->addressModeU , sizeof(s->addressModeU));
    XXH3_64bits_update(state , &s->addressModeV , sizeof(s->addressModeV));
    XXH3_64bits_update(state , &s->addressModeW , sizeof(s->addressModeW));
    XXH3_64bits_update(state , &s->mipLodBias , sizeof(s->mipLodBias));
    XXH3_64bits_update(state , &s->anisotropyEnable ,
        sizeof(s->anisotropyEnable));
    XXH3_64bits_update(state , &s->maxAnisotropy , sizeof(s->maxAnisotropy));
    XXH3_64bits_update(state , &s->compareEnable , sizeof(s->compareEnable));
    XXH3_64bits_update(state , &s->compareOp , sizeof(s->compareOp));
    XXH3_64bits_update(state , &s->minLod , sizeof(s->minLod));
    XXH3_64bits_update(state , &s->maxLod , sizeof(s->maxLod));
    XXH3_64bits_update(state , &s->borderColor , sizeof(s->borderColor));
    XXH3_64bits_update(state , &s->unnormalizedCoordinates ,
        sizeof(s->unnormalizedCoordinates));
    XXH64_hash_t const  hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}

void
destroy_vksampler_info_key(
    fht_key_t  key
) {
    mi_free((qo_pointer_t) key);
}

qo_stat_t
sampler_map_init(
    _SamplerMap * self
) {
    return fht_init(
        &self->hash_table , 1 , .1 , &(_FunnelHashTableAuxiliary) {
        .hash_func = VkSamplerCreateInfo_hash ,
        .equals_func = VkSamplerCreateInfo_compare ,
        .key_destroy_func = destroy_vksampler_info_key
    }
    );
}

void
sampler_map_destroy(
    _SamplerMap * self
) {
    fht_destroy(&self->hash_table);
}

qo_bool_t
sampler_map_insert(
    _SamplerMap *               self ,
    const VkSamplerCreateInfo * key ,
    qo_uint64_t                 value
) {
    return fht_insert(&self->hash_table , (fht_key_t) key , value);
}

qo_bool_t
sampler_map_search(
    _SamplerMap *               self ,
    const VkSamplerCreateInfo * key ,
    qo_uint64_t *               p_value
) {
    return fht_search(&self->hash_table , (fht_key_t) key ,
        (fht_value_t *) p_value);
}

qo_bool_t
sampler_map_set(
    _SamplerMap *               self ,
    const VkSamplerCreateInfo * key ,
    qo_uint64_t                 value
) {
    return fht_set(&self->hash_table , (fht_key_t) key , value);
}
