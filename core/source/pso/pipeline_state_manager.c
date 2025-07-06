#include "pipeline_state_manager.h"
#include <vulkan/vulkan_core.h>

#define DEF_DEFAULTIFY_STATE_FN(lower_snake_case , CamelCase , upper_snake_case) \
    void \
    defaultify_##lower_snake_case##_state(\
        VkPipeline##CamelCase##StateCreateInfo * p_state \
    ) { \
        memset(p_state, 0, sizeof(VkPipeline##CamelCase##StateCreateInfo)); \
        p_state->sType = VK_STRUCTURE_TYPE_PIPELINE_##upper_snake_case##_STATE_CREATE_INFO; \
    }

DEF_DEFAULTIFY_STATE_FN(rasterization, Rasterization, RASTERIZATION);
DEF_DEFAULTIFY_STATE_FN(depth_stencil , DepthStencil , DEPTH_STENCIL);
DEF_DEFAULTIFY_STATE_FN(color_blend , ColorBlend , COLOR_BLEND);

void
defaultify_color_blend_state_attachment(
    VkPipelineColorBlendAttachmentState * p_attachment
) {
    memset(p_attachment , 0 , sizeof(VkPipelineColorBlendAttachmentState));
}

qo_size_t
register_default_states(
    _PipelineStateManager * self
) {
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    qo_size_t success_count = 0;

    defaultify_rasterization_state(&rasterization_info);
    success_count += named_rasterization_state_map_insert(&self->named_rasterization_states, "default", &rasterization_info);

    defaultify_rasterization_state(&rasterization_info);
    rasterization_info.cullMode = VK_CULL_MODE_NONE;
    success_count += named_rasterization_state_map_insert(&self->named_rasterization_states, "cull_none", &rasterization_info);

    defaultify_rasterization_state(&rasterization_info);
    rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
    rasterization_info.cullMode = VK_CULL_MODE_NONE;
    success_count += named_rasterization_state_map_insert(&self->named_rasterization_states, "wireframe_no_cull", &rasterization_info);

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    defaultify_depth_stencil_state(&depth_stencil_info);
    depth_stencil_info.depthWriteEnable = VK_FALSE;
    success_count += named_depth_stencil_state_map_insert(&self->named_depth_stencil_states, "depth_test_no_write", &depth_stencil_info);

    defaultify_depth_stencil_state(&depth_stencil_info);
    depth_stencil_info.depthTestEnable = VK_FALSE;
    depth_stencil_info.depthWriteEnable = VK_FALSE;
    success_count += named_depth_stencil_state_map_insert(&self->named_depth_stencil_states, "no_depth", &depth_stencil_info);

    defaultify_depth_stencil_state(&depth_stencil_info);
    depth_stencil_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
    depth_stencil_info.depthWriteEnable = VK_FALSE;
    success_count += named_depth_stencil_state_map_insert(&self->named_depth_stencil_states, "depth_test_equal", &depth_stencil_info);

    VkPipelineColorBlendStateCreateInfo color_blend_state_info;
    defaultify_color_blend_state(&color_blend_state_info);
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    defaultify_color_blend_state_attachment(&color_blend_attachment_state);
    color_blend_state_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_info.attachmentCount = 1;
    success_count += named_color_blend_state_map_insert(&self->named_color_blend_states, "opaque", &color_blend_state_info);

    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    success_count += named_color_blend_state_map_insert(&self->named_color_blend_states, "alpha_blend", &color_blend_state_info);

    defaultify_color_blend_state_attachment(&color_blend_attachment_state);
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // 有些引擎用SRC_ALPHA
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    success_count += named_color_blend_state_map_insert(&self->named_color_blend_states, "additive", &color_blend_state_info);

    _VertexInputLayout vertex_input_layout = {};
    // TODO: Make map for it
    //success_count += vertex_input_layout_map_insert(&self->named_vertex_input_layouts, "default", &vertex_input_layout);
}