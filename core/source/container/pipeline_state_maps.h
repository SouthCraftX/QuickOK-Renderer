#pragma once
#define __QOR_PIPELINE_STATE_MAPS_SRC__

#include "funnel_hash_table.h"

#define DEF_PS_STATE_MAP_STRUCT( \
    x)  struct __ ## x ## Map { _FunnelHashTable  hash_table; }; \
        typedef struct __ ## x ## Map _ ## x ## Map

#define DECL_PS_STATE_MAP_SEARCH(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_bool_t \
        snake_case ## _map_search( \
    _ ## CamelCase ## Map * self , key_type key , value_type * value \
        )

#define DECL_PS_STATE_MAP_INSERT(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1)  \
        qo_bool_t \
        snake_case ## _map_insert( \
    _ ## CamelCase ## Map * self , key_type key , value_type value \
        )

#define DECL_PS_STATE_MAP_SET(snake_case , CamelCase , key_type , value_type) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_bool_t \
        snake_case ## _map_set( \
    _ ## CamelCase ## Map * self , key_type key , value_type value \
        )

#define DECL_PS_STATE_MAP_INIT(snake_case , CamelCase) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_stat_t \
        snake_case ## _map_init( \
    _ ## CamelCase ## Map * self \
        )

#define DECL_PS_STATE_MAP_DESTROY(snake_case , CamelCase) \
        void \
        snake_case ## _map_destroy( \
    _ ## CamelCase ## Map * self \
        )

#define DECL_PS_STATE_MAP_GET_COUNT(snake_case , CamelCase) \
        QO_NODISCARD QO_NONNULL(1) \
        qo_size_t \
        snake_case ## _map_get_count( \
    _ ## CamelCase ## Map * self \
        ) { return fht_get_count(&self->hash_table); }

#define DEF_PS_STATE_MAP(snake_case , CamelCase , key_type , value_type) \
        DEF_PS_STATE_MAP_STRUCT(CamelCase ## State); \
        DECL_PS_STATE_MAP_GET_COUNT(snake_case ## _state , CamelCase ## State) \
        DECL_PS_STATE_MAP_SEARCH(snake_case ## _state , CamelCase ## State , \
    key_type , value_type); \
        DECL_PS_STATE_MAP_INSERT(snake_case ## _state , CamelCase ## State , \
    key_type , value_type); \
        DECL_PS_STATE_MAP_SET(snake_case ## _state , CamelCase ## State , \
    key_type , value_type); \
        DECL_PS_STATE_MAP_INIT(snake_case ## _state , CamelCase ## State); \
        DECL_PS_STATE_MAP_DESTROY(snake_case ## _state , CamelCase ## State)

#define DEF_NAMED_PS_STATE_MAP(snake_case , CamelCase , value_type) \
        DEF_PS_STATE_MAP_STRUCT(Named ## CamelCase ## State); \
        DECL_PS_STATE_MAP_SEARCH(named_ ## snake_case ## _state , \
    Named ## CamelCase ## State , qo_ccstring_t , value_type); \
        DECL_PS_STATE_MAP_INSERT(named_ ## snake_case ## _state, \
    Named ## CamelCase ## State , qo_ccstring_t , value_type); \
        DECL_PS_STATE_MAP_SET(named_ ## snake_case ## _state , \
    Named ## CamelCase ## State , qo_ccstring_t , value_type); \
        DECL_PS_STATE_MAP_INIT(named_ ## snake_case ## _state , \
    Named ## CamelCase ## State); \
        DECL_PS_STATE_MAP_DESTROY(named_ ## snake_case ## _state , \
    Named ## CamelCase ## State)

DEF_PS_STATE_MAP(rasterization , Rasterization ,
    VkPipelineRasterizationStateCreateInfo * , object_id_t);
DEF_PS_STATE_MAP(depth_stencil , DepthStencil ,
    VkPipelineDepthStencilStateCreateInfo * , object_id_t);
DEF_PS_STATE_MAP(color_blend , ColorBlend ,
    VkPipelineColorBlendStateCreateInfo * , object_id_t);

DEF_NAMED_PS_STATE_MAP(rasterization , Rasterization ,
    VkPipelineRasterizationStateCreateInfo const * const);
DEF_NAMED_PS_STATE_MAP(depth_stencil , DepthStencil ,
    VkPipelineDepthStencilStateCreateInfo const * const);
DEF_NAMED_PS_STATE_MAP(color_blend , ColorBlend ,
    VkPipelineColorBlendStateCreateInfo const * const);


#undef DEF_PS_STATE_MAP
#undef DEF_NAMED_PS_STATE_MAP

#undef DECL_PS_STATE_MAP_INIT
#undef DECL_PS_STATE_MAP_DESTORY
#undef DECL_PS_STATE_MAP_GET_COUNT
#undef DECL_PS_STATE_MAP_SET
#undef DECL_PS_STATE_MAP_SEARCH
#undef DECL_PS_STATE_MAP_INSERT
