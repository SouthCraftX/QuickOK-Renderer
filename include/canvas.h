#pragma once
#define __QOR_CANVAS_H__

#include "font_engine.h"

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

struct _QO_Canvas;
typedef struct _QO_Canvas QO_Canvas;

struct _QO_Shader;
typedef struct _QO_Shader QO_Shader;

typedef enum 
{
    QO_RENDERING_SOFTWARE = 0 ,
    QO_RENDERING_EGL_D3D11 ,
    QO_RENDERING_EGL_VULKAN
} QO_RenderingBackends;

typedef enum 
{
    QO_COLORSPACE_RGB24 = 0,
    QO_COLORSPACE_RGBA32 ,
    QO_COLORSPACE_GRAY8     //< Hardware backends not implemented
} QO_Colorspace;

typedef enum 
{
    QO_CANVAS_AUTO_CLEAN = 0 ,
    QO_CANVAS_PARTIAL_CLEAN ,
    QO_CANVAS_FULL_CLEAN
} QO_CanvasCleaningMethod;

union _QO_Color
{
    struct
    {
        qo_uint8_t r ;
        qo_uint8_t g ;
        qo_uint8_t b ;
        qo_uint8_t a ;
    } rgba;

    struct
    {
        qo_uint8_t r ;
        qo_uint8_t g ;
        qo_uint8_t b ;
    } rgb;

    qo_uint8_t gray;
};
typedef union _QO_Color QO_Color;

struct _QO_RenderingEnv;
typedef struct _QO_RenderingEnv QO_RenderingEnv;

/// @brief Get the rendering environment. If not created, create it.
/// @warning In most cases, two hardware backends shouldn't be used at the same time
qo_stat_t
qo_rendering_env_get(
    QO_RenderingBackends    backend ,
    QO_Colorspace           colorspace ,
    qo_ccstring_t           hw_device_name ,//< Ignore when using software backend
    QO_RenderingEnv **      pp_env ,
    qo_bool_t               shared_between_threads //< If not, the rendering env can only be operated by the thread that created it
);

qo_bool_t
qo_is_rendering_backend_supported(
    QO_RenderingBackends  backend
);

qo_stat_t
qo_canvas_init(
    QO_Canvas *         p_canvas ,
    QO_RenderingEnv *   p_env ,
    qo_uint32_t         width ,
    qo_uint32_t         height ,
    QO_Color            default_background_color 
) QO_NONNULL(1) ;

QO_RenderingBackends
qo_get_rendering_backend();

qo_bool_t
qo_is_rendering_backend_hardware(
    QO_Canvas *        p_canvas
) QO_NONNULL(1) ;

void
qo_canvas_destroy(
    QO_Canvas *        p_canvas
);

qo_stat_t
qo_canvas_render_glyph(
    QO_Canvas *        p_canvas ,
    qo_ccstring_t       text ,
    qo_uint32_t         x ,
    qo_uint32_t         y ,
    QO_FontGlyph *     p_glyph ,
    QO_Shader *        p_shader
) QO_NONNULL(1 , 5);

void
qo_canvas_clean(
    QO_Canvas *        p_canvas ,
    QO_CanvasCleaningMethod  method
) QO_NONNULL(1);

qo_stat_t
qo_canvas_draw_rectangle(
    QO_Canvas *        p_canvas ,
    qo_uint32_t         x ,
    qo_uint32_t         y ,
    qo_uint32_t         width ,
    qo_uint32_t         height ,
    QO_Color            color
) QO_NONNULL(1) ;

qo_stat_t
qo_canvas_render_textline(
    QO_Canvas *        p_canvas ,
    qo_ccstring_t       line ,
    qo_uint32_t         x ,
    qo_uint32_t         y ,
    QO_FontEngine *    p_font_engine
) QO_NONNULL(1 , 2 , 5);

qo_stat_t
qo_rendering_env_unref(
    QO_RenderingEnv *   p_env
);

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "internal/canvas_base.h"

