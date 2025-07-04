#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_WVKSHADER_SRC__

#include "../container/vector.h"
#include "../vertex_input_layout.h"

typedef qo_uint64_t shader_id_t;

typedef enum 
{
    SHADER_NONE = 0,
    SHADER_GRAPHICS ,
    SHADER_COMPUTE
} _WVkShaderType;

struct __WVkShaderStage
{
    qo_ccstring_t   path;
    VkShaderStageFlagBits type;
};
typedef struct __WVkShaderStage _WVkShaderStage;

struct __WVkShader
{
    qo_ref_count_t      reference_count;
    _VkDeviceContext *  device_context;

    shader_id_t         shader_id;
    _WVkShaderType      shader_type;

    _Vector             stage_infos; // VkPipelineShaderStageCreateInfo
    _Vector             shader_modules; // VkShaderModule
    
    // Result of reflection
    VkPipelineLayout        pipeline_layout;
    _Vector                 descriptor_set_layouts; // VkDescriptorSetLayout
    _Vector                 push_constant_ranges; // VkPushConstantRange
    _VertexInputLayout      inferred_vertex_layout;
};
typedef struct __WVkShader _WVkShader;

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD
shader_id_t
wvkshader_get_id(
    _WVkShader *    self
) {
    return self->shader_id;
} QO_NONNULL(1)

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
VkPipelineLayout
wvkshader_get_pipeline_layout(
    _WVkShader *    self
) {
    return self->pipeline_layout;
} 

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
_WVkShaderType
wvkshader_get_type(
    _WVkShader *    self
) {
    return self->shader_type;
}

// QO_NONNULL(1 , 2 , 3)
// VkResult
// wvkshader_load_graphics(
//     _WVkShader **   p_self ,
//     _VkDeviceContext *  device_context ,
//     _WVkShaderStage *   stages ,
//     qo_uint32_t         stage_count
// );

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)
_VertexInputLayout const *
wvkshader_get_inferred_vertex_layout(
    _WVkShader *    self
) {
    return &self->inferred_vertex_layout;
} 


