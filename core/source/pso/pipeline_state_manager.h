#pragma once
#define __QOR_PSO_PIPELINE_STATE_MANAGER_SRC__

#include "../container/vector.h"
#include "../container/pipeline_state_maps.h"
#include "../vertex_input_layout.h"

struct __ExtendedDynamicStateDesc
{
    _Vector  dynamic_states;  // VkGraphicsPipelineLibraryFlagsEXT
};
typedef struct __ExtendedDynamicStateDesc _ExtendedDynamicStateDesc;

struct __PipelineStateManager
{
    _Vector                 vertex_input_layouts;
    _RasterizationStateMap  rasterization_state_cache;
    _DepthStencilStateMap   depth_stencil_state_cache;
    _ColorBlendStateMap     color_blend_state_cache;
    
    _NamedDepthStencilStateMap  named_depth_stencil_states;
    _NamedRasterizationStateMap named_rasterization_states;
    _NamedColorBlendStateMap    named_color_blend_states;
    
    object_id_t next_state_id;
};
typedef struct __PipelineStateManager _PipelineStateManager;

#define DECL_GET_STATE_ID(snake_case , CamelCase) \
        object_id_t \
        get_ ## snake_case ## _state_id( \
    _PipelineStateManager * self , \
    VkPipeline ## CamelCase ## StateCreateInfo const * const state \
        )

DECL_GET_STATE_ID(rasterization , Rasterization);
DECL_GET_STATE_ID(depth_stencil , DepthStencil);
DECL_GET_STATE_ID(color_blend , ColorBlend);

#undef DECL_GET_STATE_ID


#define DECL_GET_NAMED_STATE_FN(state_type) \
        QO_NODISCARD QO_NONNULL(1 , 2 , 3) \
        qo_bool_t \
        get_named_ ## state_type ## _state( \
    _PipelineStateManager * self , qo_ccstring_t name , \
    VkPipeline ## state_type ## StateCreateInfo * const * pp_state \
        )

DECL_GET_NAMED_STATE_FN(Rasterization);
DECL_GET_NAMED_STATE_FN(DepthStencil);
DECL_GET_NAMED_STATE_FN(ColorBlend);

#undef DECL_GET_NAMED_STATE_FN


#define DECL_GET_INDEXED_STATE_FN(state_type) \
        QO_NODISCARD QO_NONNULL(1 , 3) \
        qo_bool_t \
        get_indexed_ ## state_type ## _state( \
    _PipelineStateManager * self , object_id_t index , \
    VkPipeline ## state_type ## StateCreateInfo * const * pp_state \
        )

DECL_GET_INDEXED_STATE_FN(Rasterization);
DECL_GET_INDEXED_STATE_FN(DepthStencil);
DECL_GET_INDEXED_STATE_FN(ColorBlend);

#undef DECL_GET_INDEXED_STATE_FN


QO_NODISCARD QO_NONNULL(1 , 3)
qo_bool_t
get_extended_dynamic_state(
    _PipelineStateManager *      self ,
    object_id_t                  index ,
    _ExtendedDynamicStateDesc ** pp_extended_dynamic_state_desc
);

QO_NODISCARD QO_NONNULL(1 , 3)
qo_bool_t
get_indexed_vertex_input_layout(
    _PipelineStateManager * self ,
    object_id_t             index ,
    _VertexInputLayout *    p_vertex_input_layout
);
