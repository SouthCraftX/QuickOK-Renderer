#pragma once
#define __QOR_MATERIAL_TEMPLATE_SRC__

#include "../wrapped_vulkan_objects/wvkshader.h"
#include "../container/string_hash_material_parameter_map.h"
#include "../container/string_hash_wvktexture_map.h"

struct __MaterialTemplate
{
    material_template_id_t           id;
    _WVkShader *                     shader;
    _Vector                          tags; // strings
    _StringHashMaterialParameterMap  default_parameters;
    _StringHashWVkTextureMap         default_textures;
};
typedef struct __MaterialTemplate _MaterialTemplate;