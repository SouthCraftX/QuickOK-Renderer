#include "wvkshader.h"
#include "../../../../QuickOK-IO/QuickOK-IO/include//direct_stream.h" // TODO:
#include <mimalloc.h>
#include <spirv_cross/spirv.h>
#include <vulkan/vulkan_core.h>
#include <spirv_cross/spirv_cross_c.h>

struct __WVkShaderLoadReturns
{
    VkResult   vk_stat;
    qo_stat_t  qo_stat;
};
typedef struct __WVkShaderLoadReturns _WVkShaderLoadReturns;

#define QO_ERROR_RETURN(returns)     if (returns.qo_stat != \
                                         QO_OK) return returns
#define VK_ERROR_RETURN(returns)     if (returns.vk_stat != \
                                         VK_SUCCESS) return returns
#define RETURNS_HAVE_ERROR(returns)  (returns.qo_stat != QO_OK || \
    returns.vk_stat != VK_SUCCESS)

_WVkShaderLoadReturns
load_and_create_module(
    _WVkShader *           self ,
    qo_ccstring_t          path ,
    VkShaderStageFlagBits  stage ,
    qo_pointer_t *         p_spirv ,
    qo_size_t *            p_size
) {
    QO_DirectStream  module_stream;
    _WVkShaderLoadReturns  returns = {VK_SUCCESS , QO_OK};
    //returns.qo_stat = qo_dfile_open(&module_stream , path , strlen(path) , )
    qo_size_t  size;
    returns.qo_stat = qo_dfile_get_size(&module_stream , &size);
    QO_ERROR_RETURN(returns);

    qo_pointer_t  spirv_buffer = mi_malloc(size);
    if (!spirv_buffer)
    {
        return (_WVkShaderLoadReturns) {VK_SUCCESS , QO_OUT_OF_MEMORY};
    }

    // TODO: Handle read size
    qo_dfile_read(&module_stream , spirv_buffer , size , &returns.qo_stat);
    qo_dfile_close(&module_stream);

    VkShaderModuleCreateInfo  create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO ,
        .pNext = NULL ,
        .codeSize = size ,
        .pCode = spirv_buffer ,
        .flags = 0
    };

    VkShaderModule  shader_module;
    returns.vk_stat =
        vkCreateShaderModule(self->device_context->logical_device ,
            &create_info , get_vk_allocator() , &shader_module);
    VK_ERROR_RETURN(returns);
    if (!vector_push_back(&self->shader_modules , &shader_module))
    {
        mi_free(spirv_buffer);
        vkDestroyShaderModule(self->device_context->logical_device ,
            shader_module , get_vk_allocator());
        return (_WVkShaderLoadReturns) {VK_ERROR_OUT_OF_HOST_MEMORY , QO_OK};
    }

    VkPipelineShaderStageCreateInfo  stage_info = {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ,
        .pNext  = NULL ,
        .stage  = stage ,
        .module = shader_module
                  // Here, we don't fill pSpecializationInfo and flags
                  // We'll pass it later
    };
    if (!vector_push_back(&self->stage_infos , &stage_info))
    {
        mi_free(spirv_buffer);
        vkDestroyShaderModule(self->device_context->logical_device ,
            shader_module , get_vk_allocator());
        return (_WVkShaderLoadReturns) {VK_ERROR_OUT_OF_HOST_MEMORY , QO_OK};
    }
    *p_size  = size;
    *p_spirv = spirv_buffer;
    return (_WVkShaderLoadReturns) {VK_SUCCESS , QO_OK};
}

VkFormat
spirv_type_to_vkformat(
    spvc_type  type
) {
    spvc_basetype  basetype = spvc_type_get_basetype(type);
    unsigned  vecsize = spvc_type_get_vector_size(type);
    unsigned  width = spvc_type_get_bit_width(type);

    switch (basetype)
    {
        // --- 16-bit Types ---
        case SPVC_BASETYPE_FP16:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R16_SFLOAT;

                case 2:
                    return VK_FORMAT_R16G16_SFLOAT;

                case 3:
                    return VK_FORMAT_R16G16B16_SFLOAT;

                case 4:
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
            }
            break;

        case SPVC_BASETYPE_INT16:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R16_SINT;

                case 2:
                    return VK_FORMAT_R16G16_SINT;

                case 3:
                    return VK_FORMAT_R16G16B16_SINT;

                case 4:
                    return VK_FORMAT_R16G16B16A16_SINT;
            }
            break;

        case SPVC_BASETYPE_UINT16:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R16_UINT;

                case 2:
                    return VK_FORMAT_R16G16_UINT;

                case 3:
                    return VK_FORMAT_R16G16B16_UINT;

                case 4:
                    return VK_FORMAT_R16G16B16A16_UINT;
            }
            break;

        // --- 32-bit Types ---
        case SPVC_BASETYPE_FP32:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R32_SFLOAT;

                case 2:
                    return VK_FORMAT_R32G32_SFLOAT;

                case 3:
                    return VK_FORMAT_R32G32B32_SFLOAT;

                case 4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            break;

        case SPVC_BASETYPE_INT32:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R32_SINT;

                case 2:
                    return VK_FORMAT_R32G32_SINT;

                case 3:
                    return VK_FORMAT_R32G32B32_SINT;

                case 4:
                    return VK_FORMAT_R32G32B32A32_SINT;
            }
            break;

        case SPVC_BASETYPE_UINT32:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R32_UINT;

                case 2:
                    return VK_FORMAT_R32G32_UINT;

                case 3:
                    return VK_FORMAT_R32G32B32_UINT;

                case 4:
                    return VK_FORMAT_R32G32B32A32_UINT;
            }
            break;

        // --- 64-bit Types ---
        case SPVC_BASETYPE_FP64:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R64_SFLOAT;

                case 2:
                    return VK_FORMAT_R64G64_SFLOAT;

                case 3:
                    return VK_FORMAT_R64G64B64_SFLOAT;

                case 4:
                    return VK_FORMAT_R64G64B64A64_SFLOAT;
            }
            break;

        case SPVC_BASETYPE_INT64:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R64_SINT;

                case 2:
                    return VK_FORMAT_R64G64_SINT;

                case 3:
                    return VK_FORMAT_R64G64B64_SINT;

                case 4:
                    return VK_FORMAT_R64G64B64A64_SINT;
            }
            break;

        case SPVC_BASETYPE_UINT64:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R64_UINT;

                case 2:
                    return VK_FORMAT_R64G64_UINT;

                case 3:
                    return VK_FORMAT_R64G64B64_UINT;

                case 4:
                    return VK_FORMAT_R64G64B64A64_UINT;
            }
            break;

        // --- 8-bit Types ---
        case SPVC_BASETYPE_INT8:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R8_SINT;

                case 2:
                    return VK_FORMAT_R8G8_SINT;

                case 3:
                    return VK_FORMAT_R8G8B8_SINT;

                case 4:
                    return VK_FORMAT_R8G8B8A8_SINT;
            }
            break;

        case SPVC_BASETYPE_UINT8:
            switch (vecsize)
            {
                case 1:
                    return VK_FORMAT_R8_UINT;

                case 2:
                    return VK_FORMAT_R8G8_UINT;

                case 3:
                    return VK_FORMAT_R8G8B8_UINT;

                case 4:
                    return VK_FORMAT_R8G8B8A8_UINT;
            }
            break;

        default:
            // 其他类型如 struct, sampler, image, bool 等不直接映射到顶点格式
            return VK_FORMAT_UNDEFINED;
    }

    // 如果 switch 中有 case 没有 return，则会执行到这里
    return VK_FORMAT_UNDEFINED;
}

VkResult
reflect(
    _WVkShader *    self ,
    qo_pointer_t * spirv_blobs ,
    qo_size_t *    spirv_sizes,
    qo_uint32_t    count
) {
    memset(&self->inferred_vertex_layout , 0 , sizeof(self->inferred_vertex_layout));
    vector_clear(&self->push_constant_ranges);

    spvc_context context = NULL;
    spvc_context_create(&context);

    for (qo_uint32_t i = 0 ; i < count ; ++i)
    {
        qo_pointer_t spirv = spirv_blobs[i];
        VkPipelineShaderStageCreateInfo * stage_info = vector_at(&self->stage_infos , i);
        VkShaderStageFlagBits stage = stage_info->stage;

        spvc_parsed_ir ir;
        spvc_context_parse_spirv(context, spirv, spirv_sizes[i] , &ir);
        
        spvc_compiler compiler = NULL;
        spvc_context_create_compiler(context , SPVC_BACKEND_NONE , ir , SPVC_CAPTURE_MODE_TAKE_OWNERSHIP , &compiler);

        spvc_resources resources = NULL;
        spvc_compiler_create_shader_resources(compiler , &resources);

        if (stage == VK_SHADER_STAGE_VERTEX_BIT)
        {
            spvc_reflected_resource const * input_list = NULL;
            qo_size_t input_count;
            spvc_resources_get_resource_list_for_type(resources , SPVC_RESOURCE_TYPE_STAGE_INPUT , &input_list , &input_count);

            typedef struct 
            {
                qo_uint32_t location;
                spvc_type   type;
            } TempAttribute;
            _Vector temp_attributes;
            if (!vector_init(&temp_attributes, sizeof(TempAttribute)))
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }

            for (qo_size_t j = 0 ; j < input_count ; ++j)
            {
                spvc_reflected_resource  resource = input_list[j];
                SpvStorageClass storage_class;
                spvc_type type = spvc_compiler_get_type_handle(compiler, resource.type_id);
                storage_class = spvc_type_get_storage_class(type);
                if (storage_class != SpvStorageClassInput)
                {
                    continue;
                }

                qo_uint32_t location = spvc_compiler_get_decoration(compiler, resource.id, SpvDecorationLocation);
                if (!vector_push_back(&temp_attributes, (qo_cpointer_t)&(TempAttribute){location, type}))
                {
                    return VK_ERROR_OUT_OF_HOST_MEMORY;
                }
            }

            // TODO: sort

            qo_uint32_t stride = 0;
            for (_VectorIterator now = vector_iterate_begin(&temp_attributes) , end = vector_iterate_end(&temp_attributes);
                 !vector_iterator_equals(&now, &end);
                 vector_iterator_next(&now))
            {
                TempAttribute * this_attr = vector_iterator_get(&now);
                VkVertexInputAttributeDescription attr_desc = {
                    .location = this_attr->location ,
                    .binding = 0 ,
                    .format = spirv_type_to_vkformat(this_attr->type),
                    .offset = stride
                };
                // TODO: push back
                unsigned width = spvc_type_get_bit_width(this_attr->type);
                unsigned vecsize = spvc_type_get_vector_size(this_attr->type);
                unsigned columns = spvc_type_get_columns(this_attr->type);
                stride += (width / 8) * vecsize * columns;
            }

            // TODO: m_inferredVertexLayout.attributes.empty
            
            
        }
    }
}

_WVkShaderLoadReturns
wvkshader_load_graphics(
    _WVkShader **      p_self ,
    _VkDeviceContext * device_context ,
    _WVkShaderStage *  stages ,
    qo_uint32_t        stage_count
) {
    _WVkShader * self = mi_malloc_tp(_WVkShader);
    if (!self)
    {
        return (_WVkShaderLoadReturns) {VK_SUCCESS , QO_OUT_OF_MEMORY};
    }

    QO_ASSERT(stage_count < 255);
    qo_pointer_t  spirv_blobs[stage_count];
    qo_size_t  spirv_sizes[stage_count];
    for (qo_int32_t i = 0 ; i < stage_count ; ++i)
    {
        _WVkShaderLoadReturns  returns = load_and_create_module(
            self , stages[i].path , stages[i].type , &spirv_blobs[i] ,
            &spirv_sizes[i]
        );
        if (RETURNS_HAVE_ERROR(returns))
        {
            for (qo_int32_t j = 0 ; j < i ; ++j)
            {
                mi_free(spirv_blobs[j]);
                vkDestroyShaderModule(self->device_context->logical_device ,
                    self->shader_modules[j] , get_vk_allocator());
            }
            mi_free(self);
            return returns;
        }
    }
}
