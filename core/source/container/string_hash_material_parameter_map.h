#pragma once
#define __QOR_STRING_HASH_MATERIAL_PARAMETER_MAP_SRC__

#include "funnel_hash_table.h"
#include <cglm/cglm.h>

typedef qo_uint64_t material_template_id_t;

typedef enum
{
    MATERIAL_PARAMETER_FP32 ,
    MATERIAL_PARAMETER_I32 ,
    MATERIAL_PARAMETER_VEC2 ,
    MATERIAL_PARAMETER_VEC3 ,
    MATERIAL_PARAMETER_VEC4
} _MaterialPatameterType;

struct __MaterialPatameter
{
    _MaterialPatameterType  type;
    union
    {
        qo_fp32_t   fp32;
        qo_int32_t  i32;
        vec2        v2;
        vec3        v3;
        vec4        v4;
    } data;
};

typedef struct __MaterialPatameter _MaterialPatameter;

struct __StringHashMaterialParameterMap
{
    _FunnelHashTable  table;
};

typedef struct __StringHashMaterialParameterMap _StringHashMaterialParameterMap;

QO_NODISCARD  QO_NONNULL(1)
qo_stat_t
string_hash_material_parameter_map_init(
    _StringHashMaterialParameterMap * self
);

void
string_hash_material_parameter_map_destroy(
    _StringHashMaterialParameterMap * self
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_material_parameter_map_search(
    _StringHashMaterialParameterMap * self ,
    string_hash_t                     string_hash ,
    _MaterialPatameter *              p_parameter
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_material_parameter_map_insert(
    _StringHashMaterialParameterMap * self ,
    string_hash_t                     string_hash ,
    _MaterialPatameter *              p_parameter
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_material_parameter_map_set(
    _StringHashMaterialParameterMap * self ,
    string_hash_t                     string_hash ,
    _MaterialPatameter const *        parameter
);
