#include "pipeline_state_maps.h"
#include "funnel_hash_table.h"

XXH64_hash_t
VkPipelineRasterizationStateCreateInfo_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    VkPipelineRasterizationStateCreateInfo const * const  s =
        (VkPipelineRasterizationStateCreateInfo const *) key;
    XXH3_state_t * const  state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);
    XXH3_64bits_update(state , &s->flags , sizeof(s->flags));
    XXH3_64bits_update(state , &s->depthClampEnable ,
        sizeof(s->depthClampEnable));
    XXH3_64bits_update(state , &s->rasterizerDiscardEnable ,
        sizeof(s->rasterizerDiscardEnable));
    XXH3_64bits_update(state , &s->polygonMode , sizeof(s->polygonMode));
    XXH3_64bits_update(state , &s->cullMode , sizeof(s->cullMode));
    XXH3_64bits_update(state , &s->frontFace , sizeof(s->frontFace));
    XXH3_64bits_update(state , &s->depthBiasEnable ,
        sizeof(s->depthBiasEnable));
    XXH3_64bits_update(state , &s->depthBiasSlopeFactor ,
        sizeof(s->depthBiasSlopeFactor));
    XXH3_64bits_update(state , &s->depthBiasConstantFactor ,
        sizeof(s->depthBiasConstantFactor));
    XXH3_64bits_update(state , &s->depthBiasClamp , sizeof(s->depthBiasClamp));
    XXH3_64bits_update(state , &s->lineWidth , sizeof(s->lineWidth));
    XXH64_hash_t  hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}

XXH64_hash_t
VkPipelineDepthStencilStateCreateInfo_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    VkPipelineDepthStencilStateCreateInfo const * const  s =
        (VkPipelineDepthStencilStateCreateInfo const *) key;
    XXH3_state_t * const  state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);
    XXH3_64bits_update(state , &s->flags , sizeof(s->flags));
    XXH3_64bits_update(state , &s->depthTestEnable ,
        sizeof(s->depthTestEnable));
    XXH3_64bits_update(state , &s->depthWriteEnable ,
        sizeof(s->depthWriteEnable));
    XXH3_64bits_update(state , &s->depthCompareOp , sizeof(s->depthCompareOp));
    XXH3_64bits_update(state , &s->depthBoundsTestEnable ,
        sizeof(s->depthBoundsTestEnable));
    XXH3_64bits_update(state , &s->stencilTestEnable ,
        sizeof(s->stencilTestEnable));
    XXH3_64bits_update(state , &s->front , sizeof(s->front));
    XXH3_64bits_update(state , &s->back , sizeof(s->back));
    XXH3_64bits_update(state , &s->minDepthBounds , sizeof(s->minDepthBounds));
    XXH3_64bits_update(state , &s->maxDepthBounds , sizeof(s->maxDepthBounds));
}

void
VkPipelineColorBlendAttachmentState_blend_hash(
    VkPipelineColorBlendAttachmentState const * s ,
    qo_uint32_t                                 count ,
    XXH3_state_t *  const                       state
) {
    for (VkPipelineColorBlendAttachmentState const * end =
             s + count ; s != end ;
         ++s)
    {
        XXH3_64bits_update(state , &s->blendEnable , sizeof(s->blendEnable));
        XXH3_64bits_update(state , &s->srcColorBlendFactor ,
            sizeof(s->srcColorBlendFactor));
        XXH3_64bits_update(state , &s->dstColorBlendFactor ,
            sizeof(s->dstColorBlendFactor));
        XXH3_64bits_update(state , &s->colorBlendOp , sizeof(s->colorBlendOp));
        XXH3_64bits_update(state , &s->srcAlphaBlendFactor ,
            sizeof(s->srcAlphaBlendFactor));
        XXH3_64bits_update(state , &s->dstAlphaBlendFactor ,
            sizeof(s->dstAlphaBlendFactor));
        XXH3_64bits_update(state , &s->alphaBlendOp , sizeof(s->alphaBlendOp));
        XXH3_64bits_update(state , &s->colorWriteMask ,
            sizeof(s->colorWriteMask));
    }
}

XXH64_hash_t
VkPipelineColorBlendStateCreateInfo_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    VkPipelineColorBlendStateCreateInfo const * const  s =
        (VkPipelineColorBlendStateCreateInfo const *) key;
    XXH3_state_t * const  state = XXH3_createState();
    XXH3_64bits_reset_withSeed(state , salt);
    XXH3_64bits_update(state , &s->flags , sizeof(s->flags));
    XXH3_64bits_update(state , &s->logicOpEnable , sizeof(s->logicOpEnable));
    XXH3_64bits_update(state , &s->logicOp , sizeof(s->logicOp));
    XXH3_64bits_update(state , &s->attachmentCount ,
        sizeof(s->attachmentCount));
    VkPipelineColorBlendAttachmentState_blend_hash(s->pAttachments ,
        s->attachmentCount , state);
    XXH3_64bits_update(state , &s->blendConstants , sizeof(s->blendConstants));
    XXH64_hash_t  hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
}

qo_bool_t
VkPipelineRasterizationStateCreateInfo_compare(
    fht_key_t  x ,
    fht_key_t  y
) {
    VkPipelineRasterizationStateCreateInfo const * const  sx =
        (VkPipelineRasterizationStateCreateInfo const *) x;
    VkPipelineRasterizationStateCreateInfo const * const  sy =
        (VkPipelineRasterizationStateCreateInfo const *) y;
    return sx->depthClampEnable == sy->depthClampEnable &&
           sx->rasterizerDiscardEnable == sy->rasterizerDiscardEnable &&
           sx->polygonMode == sy->polygonMode && sx->cullMode == sy->cullMode &&
           sx->frontFace == sy->frontFace &&
           sx->depthBiasEnable == sy->depthBiasEnable &&
           sx->lineWidth == sy->lineWidth &&
           sx->depthBiasConstantFactor == sy->depthBiasConstantFactor &&
           sx->depthBiasClamp == sy->depthBiasClamp &&
           sx->depthBiasSlopeFactor == sy->depthBiasSlopeFactor &&
           sx->flags == sy->flags;
}

qo_bool_t
VkStencilOpState_compare(
    VkStencilOpState const * x ,
    VkStencilOpState const * y
) {
    return x->failOp == y->failOp && x->passOp == y->passOp &&
           x->depthFailOp == y->depthFailOp && x->compareOp == y->compareOp &&
           x->compareMask == y->compareMask && x->writeMask == y->writeMask &&
           x->reference == y->reference;
}

qo_bool_t
VkPipelineDepthStencilStateCreateInfo_compare(
    fht_key_t  x ,
    fht_key_t  y
) {
    VkPipelineDepthStencilStateCreateInfo const * const  sx =
        (VkPipelineDepthStencilStateCreateInfo const *) x;
    VkPipelineDepthStencilStateCreateInfo const * const  sy =
        (VkPipelineDepthStencilStateCreateInfo const *) y;
    return sx->flags == sy->flags &&
           sx->depthTestEnable == sy->depthTestEnable &&
           sx->depthWriteEnable == sy->depthWriteEnable &&
           sx->depthCompareOp == sy->depthCompareOp &&
           sx->depthBoundsTestEnable == sy->depthBoundsTestEnable &&
           sx->stencilTestEnable == sy->stencilTestEnable &&
           VkStencilOpState_compare(&sx->front ,
        &sy->front) && VkStencilOpState_compare(&sx->back ,
        &sy->back) && sx->minDepthBounds == sy->minDepthBounds &&
           sx->maxDepthBounds == sy->maxDepthBounds;
}

qo_bool_t
VkPipelineColorBlendAttachmentState_compare(
    VkPipelineColorBlendAttachmentState const * x ,
    VkPipelineColorBlendAttachmentState const * y ,
    qo_uint32_t                                 x_count ,
    qo_uint32_t                                 y_count
) {
    if (x_count != y_count)
    {
        return qo_false;
    }

    for (qo_uint32_t i = 0 ; i < x_count ; ++i)
    {
        qo_bool_t  once = x->blendEnable == y->blendEnable &&
                          x->srcColorBlendFactor == y->srcColorBlendFactor &&
                          x->dstColorBlendFactor == y->dstColorBlendFactor &&
                          x->colorBlendOp == y->colorBlendOp &&
                          x->srcAlphaBlendFactor == y->srcAlphaBlendFactor &&
                          x->dstAlphaBlendFactor == y->dstAlphaBlendFactor &&
                          x->alphaBlendOp == y->alphaBlendOp &&
                          x->colorWriteMask == y->colorWriteMask;
        if (!once)
        {
            return qo_false;
        }
    }
    return qo_true;
}

qo_bool_t
VkPipelineColorBlendStateCreateInfo_compare(
    fht_key_t  x ,
    fht_key_t  y
) {
    VkPipelineColorBlendStateCreateInfo const * const  sx =
        (VkPipelineColorBlendStateCreateInfo const *) x;
    VkPipelineColorBlendStateCreateInfo const * const  sy =
        (VkPipelineColorBlendStateCreateInfo const *) y;
    return sx->blendConstants == sy->blendConstants &&
           sx->logicOpEnable == sy->logicOpEnable &&
           sx->logicOp == sy->logicOp &&
           sx->attachmentCount == sy->attachmentCount &&
           sx->flags == sy->flags &&
           VkPipelineColorBlendAttachmentState_compare(sx->pAttachments ,
        sy->pAttachments , sx->attachmentCount , sy->attachmentCount);
}

#define DEF_PS_MAP_INIT(snake_case , CamelCase , hash_fn , compare_fn) \
        qo_stat_t \
        snake_case ## _map_init( \
    _ ## CamelCase ## Map * self \
        ) { \
            return fht_init( \
    &self->hash_table , 0 , 0.1 , &(_FunnelHashTableAuxiliary) { \
            .hash_func = hash_fn , \
            .equals_func = compare_fn , \
            .key_destroy_func = NULL \
        } \
            ); \
        }

#define DEF_PS_MAP_DESTROY(snake_case , CamelCase) \
        void \
        snake_case ## _map_destroy( \
    _ ## CamelCase ## Map * self \
        ) { \
            return fht_destroy(&self->hash_table); \
        }

#define DEF_PS_MAP_SEARCH(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_bool_t \
        snake_case ## _map_search( \
    _ ## CamelCase ## Map * self , key_type key , value_type * p_value \
        ) { \
            return fht_search( \
    &self->hash_table , (fht_key_t) key , (fht_value_t *) p_value \
            ); \
        }

#define DEF_PS_MAP_INSERT(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1)  \
        qo_bool_t \
        snake_case ## _map_insert( \
    _ ## CamelCase ## Map * self , key_type key , value_type value \
        ) { \
            return fht_insert( \
    &self->hash_table , (fht_key_t) key , value \
            ); \
        }

#define DEF_PS_MAP_SET(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_bool_t \
        snake_case ## _map_set( \
    _ ## CamelCase ## Map * self , key_type key , value_type value \
        ) { \
            return fht_set( \
    &self->hash_table , (fht_key_t) key , value \
            ); \
        }

#define DEF_PS_MAP_FUNC(snake_case , CamelCase , key_type , value_type , \
                        hash_fn , compare_fn) \
        DEF_PS_MAP_INIT(snake_case , CamelCase , hash_fn , compare_fn); \
        DEF_PS_MAP_DESTROY(snake_case , CamelCase); \
        DEF_PS_MAP_SEARCH(snake_case , CamelCase , key_type , value_type); \
        DEF_PS_MAP_INSERT(snake_case , CamelCase , key_type , value_type); \
        DEF_PS_MAP_SET(snake_case , CamelCase , key_type , value_type)

DEF_PS_MAP_FUNC(rasterization_state , RasterizationState ,
    VkPipelineRasterizationStateCreateInfo * , object_id_t ,
    VkPipelineRasterizationStateCreateInfo_hash ,
    VkPipelineRasterizationStateCreateInfo_compare);

DEF_PS_MAP_FUNC(depth_stencil_state , DepthStencilState ,
    VkPipelineDepthStencilStateCreateInfo * , object_id_t ,
    VkPipelineDepthStencilStateCreateInfo_hash ,
    VkPipelineDepthStencilStateCreateInfo_compare);

DEF_PS_MAP_FUNC(color_blend_state , ColorBlendState ,
    VkPipelineColorBlendStateCreateInfo * , object_id_t ,
    VkPipelineColorBlendStateCreateInfo_hash ,
    VkPipelineColorBlendStateCreateInfo_compare);


