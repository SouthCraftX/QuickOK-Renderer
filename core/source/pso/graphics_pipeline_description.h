#pragma once
#define __QOR_GRAPHICS_PIPELINE_DESCRIPTION_SRC__

#include "../container/vector.h"



struct __PNextLink
{
    object_id_t extension_name_id; // Hash
    object_id_t state_id;
};
typedef struct __PNextLink _PNextLink;  

struct __GraphicsPipelineDescription
{
    object_id_t shader_id;
    object_id_t render_pass_id;
    qo_uint32_t subpass;

    object_id_t vertex_input_state_id;
    object_id_t input_assembly_state_id;
    object_id_t rasterizer_state_id;
    object_id_t depth_stencil_state_id;
    object_id_t color_blend_state_id;
    object_id_t multisample_state_id;
    object_id_t dynamic_state_id;

    _Vector     extension_states; // We need ordered
};
typedef struct __GraphicsPipelineDescription _GraphicsPipelineDescription;