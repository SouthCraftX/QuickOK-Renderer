#pragma once
#define __QOR_FONT_ENGINE_H__

#define QO_ENABLE_EXPERIMENTAL_CXX

#include "../../QOZero/include/qozero.h"
#if defined(__cplusplus)

struct _QOR_FontEngine;
typedef _QOR_FontEngine QO_FontEngine;

struct _QOR_FontGlyph;
typedef _QOR_FontGlyph QO_FontGlyph;

typedef enum 
{
    QO_FONT_ENGINE_FREETYPE = 0 ,
    QO_FONT_ENGINE_STB_TRUETYPE ,
    QO_FONT_ENGINE_GDI , // Windows only
} QO_FontEngineBackend; //__qo_font_engine_backend;

typedef enum
{
    QOR_FONT_SIZE_PIXELS = 0,
    QOR_FONT_SIZE_1P64POINTS // 1/64th of a point
} QO_FontSizeUnit;

qo_stat_t
qo_font_engine_new(
    QO_FontEngine **       pp_font_engine ,
    qo_ccstring_t           ttf_path ,
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

/// @brief Set cached glyph count for the font engine
/// @param font_engine Font engine to set cache for
/// @param cached_glyph_count Number of glyphs to cache. 0 disables caching them
///        If the font engine don't support generating glyphs, this parameter is
///        ignored.
/// @param cached_bitmap_count Number of bitmaps to cache. 0 disables caching them
/// @return The status of the operation
/// @retval QO_OK The operation was successful
/// @retval QO_OUT_OF_MEMORY No enough memory to reserve specified number of glyphs
qo_stat_t
qo_font_engine_set_cache(
    QO_FontEngine *        font_engine ,
    qo_uint32_t             cached_glyph_count ,
    qo_uint32_t             cached_bitmap_count
) QO_NONNULL(1);

qo_stat_t
qo_font_engine_cache_glyphs(
    QO_FontEngine *        font_engine ,
    qo_ccstring_t           utf8_string
) QO_NONNULL(1);

/// @brief Clean the cache of the font engine
/// @param font_engine Font engine to clean cache for
/// @param clean_glyphs Whether to clean cached glyphs
/// @param clean_bitmaps Whether to clean cached bitmaps
/// @return The status of the operation
qo_stat_t
qo_font_engine_clean_cache(
    QO_FontEngine *        font_engine ,
    qo_bool_t               clean_glyphs ,
    qo_bool_t               clean_bitmaps
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
qo_font_engine_render_bitmap(
    QO_FontEngine *        font_engine 

); 

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

#endif // __cplusplus
