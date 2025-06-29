#pragma once
#define __QOR_CHAR_RENDERER_GLYPH_PROVIDER_SRC__

#include "../rendering_env.h"

typedef enum
{
    GLYPH_RASTERIZATION_MODE_BITMAP ,
    GLYPH_RASTERIZATION_MODE_SDF ,
    GLYPH_RASTERIZATION_MODE_MSDF
} _GlyphRasterizationMode;

struct __RawGlyphData
{
    qo_byte_t *              pixel_data;
    VkExtent2D               extent;
    _GlyphRasterizationMode  glyph_rasterization_mode;
};
typedef struct __RawGlyphData _RawGlyphData;


struct __FontFaceInfo
{
    qo_cstring_t  path;
    qo_uint32_t   face_index; // .ttc/.otc may contain mutilple faces
    qo_uint32_t   pixel_size;
};
typedef struct __FontFaceInfo   _FontFaceInfo;

typedef struct __IGlyphProvider _IGlyphProvider;
struct __IGlyphProvider
{
    qo_stat_t
    (* load_font_face)(
        _IGlyphProvider * self ,
        _FontFaceInfo *   font_face_info
    );

    qo_bool_t
    (* get_glyph_data)(
        _IGlyphProvider *        self ,
        qo_uint32_t              glyph_index ,
        _GlyphRasterizationMode  glyph_rasterization_mode ,
        _RawGlyphData *          p_glyph_data
    );

    
};
