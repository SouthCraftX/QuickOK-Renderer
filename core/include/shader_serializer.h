#pragma once
#define _QO_SHADER_SERIALIZER_H__

#define QO_ENABLE_EXPERIMENTAL_CXX
#include "../../QOZero/include/qozero.h"
#include "renderer.h"

#include <GLES3/gl3.h>

// You must cache shader before program
// The serialization follows the standard QORenderer's OpenGL ES cache format
// @see doc/
struct _QO_GLESSerializer;
typedef struct _QO_GLESSerializer QO_ShaderSerializer;

typedef enum 
{
    QO_SERIALIZATION_SECTION_SHADER ,
    QO_SERIALIZATION_SECTION_PROGRAM ,
    QO_SERIALIZATION_SECTION_VBO ,
    QO_SERIALIZATION_SECTION_EBO ,
    QO_SERIALIZATION_SECTION_UBO ,
    QO_SERIALIZATION_SECTION_SSBO ,
    QO_SERIALIZATION_SECTION_TEXTURE //< KTX or DDS, not implemented yet
} QO_ShaderSerializerSection;

qo_stat_t
qo_gles_serializer_init(
    QO_ShaderSerializer *   p_serializer ,
    qo_ccstring_t           out_file_path ,
    qo_ccstring_t           device_name ,
    qo_size_t               device_name_length
) QO_NONNULL(1 , 2);

qo_stat_t
qo_gles_serializer_switch_to_section(
    QO_ShaderSerializer *       p_serializer ,
    QO_ShaderSerializerSection  section
) QO_NONNULL(1);

qo_stat_t
qo_gles_serialize_object(
    QO_ShaderSerializer *       p_serializer ,
    qo_pointer_t                object , //< must be binary, not GLES handles
    qo_size_t                   object_size ,
    qo_ccstring_t               object_name ,
    qo_size_t                   object_name_length
) QO_NONNULL(1 , 2 , 4);

void
qo_gles_serializer_destroy(
    QO_ShaderSerializer *   p_serializer 
);