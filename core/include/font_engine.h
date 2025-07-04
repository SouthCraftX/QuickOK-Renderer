#pragma once
#define __QOR_FONT_ENGINE_H__

#define QO_ENABLE_EXPERIMENTAL_CXX

#include "../../../QuickOK-Zero/include/qozero.h"
#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

struct _QO_FontEngine;
typedef _QO_FontEngine QO_FontEngine;

struct _QO_FontGlyph;
typedef _QO_FontGlyph QO_FontGlyph;

typedef enum 
{
    QO_FONT_ENGINE_FREETYPE = 0 ,
    QO_FONT_ENGINE_STB_TRUETYPE ,
    QO_FONT_ENGINE_DIRECTWRITE , // Windows only
} QO_FontEngineBackend; //__qo_font_engine_backend;

typedef enum
{
    QOR_FONT_SIZE_PIXELS = 0,
    QOR_FONT_SIZE_1P64POINTS // 1/64th of a point
} QO_FontSizeUnit;

qo_stat_t
qo_font_engine_new(
    QO_FontEngine **       pp_font_engine ,
    qo_ccstring_t           font_path ,
    QO_FontEngineBackend   backend
) QO_NONNULL(1 , 2);

qo_stat_t
qo_font_engine_new_from_buffer(
    QO_FontEngine **       pp_font_engine ,
    qo_pointer_t            font_buffer ,
    qo_size_t               font_buffer_size ,
    QO_FontEngineBackend   backend
) QO_NONNULL(1 , 2);

qo_bool_t
qo_font_engine_can_generate_glyphs(
    QO_FontEngine *        font_engine
) QO_NONNULL(1);

qo_stat_t
qo_font_engine_set_char_size(
    QO_FontEngine *        font_engine ,
    qo_uint32_t             width ,
    qo_uint32_t             height ,
    QO_FontSizeUnit        unit
) QO_NONNULL(1);



/*
qo_stat_t
qo_font_engine_set_transformation(
    QOR_FontEngine *        font_engine ,
    qo_uint32_t             xx ,
    qo_uint32_t             xy ,
    qo_uint32_t             yx ,
    qo_uint32_t             yy
) QO_NONNULL(1);
*/

qo_stat_t
qo_font_engine_render_utf8char_bitmap(
    QO_FontEngine *        font_engine ,
    qo_ccstring_t          utf8_char   
) QO_NONNULL(1, 2);

qo_stat_t
qo_font_engine_render_utf16char_bitmap(
    QO_FontEngine *        font_engine,
    char16_t *             utf16_char // TODO: replcae with qo_cwcstring_t
) QO_NONNULL(1, 2);

qo_stat_t
qo_font_glyph_to_bitmap(
    QO_FontGlyph *        font_glyph ,
    qo_pointer_t           bitmap ,
    qo_uint32_t            bitmap_width ,
    qo_uint32_t            bitmap_height
);

void
qo_font_engine_unref(
    QO_FontEngine *        font_engine
);

void
qo_font_glyph_unref(
    QO_FontGlyph *        font_glyph
);

#if defined (__cplusplus)
}
#endif // __cplusplus
