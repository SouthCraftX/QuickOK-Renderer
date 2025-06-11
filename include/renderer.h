#pragma once
#define __QO_RENDERER_H__

#include "font_engine.h"

#include <vulkan/vulkan.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

struct _QO_Canvas;
typedef struct _QO_Canvas QO_Canvas;

struct _QO_Shader;
typedef struct _QO_Shader QO_Shader;

struct _QO_RenderingDevice;
typedef struct _QO_RenderingDevice QO_RenderingDevice;

typedef enum 
{
    QO_RENDERING_ANGLE_SWIFTSHADER ,
    QO_RENDERING_ANGLE_D3D11 ,
    QO_RENDERING_ANGLE_VULKAN
} QO_RenderingBackends;

typedef enum 
{
    QO_COLORSPACE_RGB24 = 0,
    QO_COLORSPACE_RGBA32 ,
    QO_COLORSPACE_GRAY8     //< Hardware backends not implemented (THIS COMMENT IS OUTDATED)
} QO_Colorspace;

typedef enum 
{
    QO_CANVAS_AUTO_CLEAN = 0 ,
    QO_CANVAS_PARTIAL_CLEAN ,
    QO_CANVAS_FULL_CLEAN
} QO_CanvasCleaningMethod;

struct _QO_Rectangle
{
    qo_int32_t x;
    qo_int32_t y;
    qo_int32_t width;
    qo_int32_t height;
};
typedef struct _QO_Rectangle QO_Rectangle;

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

struct _QO_RenderingDeviceIterator;
typedef struct _QO_RenderingDeviceIterator QO_RenderingDeviceIterator;

QO_RenderingDeviceIterator *
qo_rendering_device_iterator_new(
    QO_RenderingBackends    backend
);

/// @param hw_device_name will be ignored if backend is not hardware. Adding '!'
///        prefix to the name indicates the rest of the string is manufacturer,
///        like "!NVIDIA" or "!AMD". Adding '?' prefix indicates the rest of the
///        string is a part of the name, like "?3060" or "?Radeon". The matching
///        is case-insensitive. If no special prefix is used, `hw_device_name`
///        will be taken as full name. If `hw_device_name` is NULL, high
///        performance device will be used.
QO_RenderingDevice *
qo_find_rendering_device(
    QO_RenderingDeviceIterator *    p_iterator ,
    qo_ccstring_t                   hw_device_name
) QO_NONNULL(1 , 2);

QO_NO_SIDE_EFFECTS
qo_ccstring_t
qo_rendering_device_get_name(
    QO_RenderingDevice *    p_device
) QO_NONNULL(1);

void
qo_rendering_device_iterator_reset(
    QO_RenderingDeviceIterator *    p_iterator
) QO_NONNULL(1);

QO_NO_SIDE_EFFECTS
QO_RenderingDevice *
qo_rendering_device_iterator_begin(
    QO_RenderingDeviceIterator *    p_iterator
) QO_NONNULL(1);

QO_NO_SIDE_EFFECTS
QO_RenderingDevice *
qo_rendering_device_iterator_next(
    QO_RenderingDeviceIterator *    p_iterator
) QO_NONNULL(1);

QO_NO_SIDE_EFFECTS
QO_RenderingDevice *
qo_rendering_device_iterator_end(
    QO_RenderingDeviceIterator *    p_iterator
) QO_NONNULL(1);

void
qo_rendering_device_iterator_unref(
    QO_RenderingDeviceIterator *    p_iterator
);

#define QO_ITERATE_RENDERING_DEVICES(p_iterator , p_device) \
    for (p_device = qo_rendering_device_iterator_begin(p_iterator) ; \
         p_device != qo_rendering_device_iterator_end(p_iterator) ; \
         p_device = qo_rendering_device_iterator_next(p_iterator) )

/// @brief Get the rendering environment. If not created, create it.
/// @return The status of the operation.
/// @retval QO_NOT_SUPPORTED Something not supported found when initializing.
///         Call qo_rendering_env_get_message() for more information.
/// @retval QO_OUT_OF_MEMORY NO sufficient memory.
/// @retval QO_MISMATCH A rendering environment already exists with different
///         colorspace or backend with specified arguments.
/// @warning Currently only one rendering environment can be created per thread.
/// @sa `qo_rendering_env_unref()`
qo_stat_t
qo_rendering_env_get(
    QO_Colorspace           colorspace ,
    QO_RenderingDevice *    p_device ,
    QO_RenderingEnv **      pp_env 
);

QO_RenderingBackends
qo_rendering_env_get_type(
    QO_RenderingEnv *   p_env
);

qo_stat_t
qo_rendering_env_unref(
    QO_RenderingEnv *   p_env
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

qo_bool_t
qo_is_rendering_backend_hardware(
    QO_Canvas *        p_canvas
) QO_NONNULL(1) ;

void
qo_canvas_destroy(
    QO_Canvas *        p_canvas
);

qo_stat_t
qo_canvas_bind_font_engine(
    QO_Canvas *        p_canvas ,
    QO_FontEngine *    p_font_engine
) QO_NONNULL(1 , 2);

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
    QO_Canvas *             p_canvas ,
    QO_CanvasCleaningMethod method
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
    qo_uint32_t         y 
) QO_NONNULL(1 , 2);


#if defined(__cplusplus)
}
#endif // __cplusplus

#include "internal/canvas_base.h"
#include "internal/rendering_device_common.h"
