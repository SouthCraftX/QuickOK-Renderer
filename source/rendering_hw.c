// #include <libavcodec/avcodec.h>
// #include <libavcodec/codec.h>
// #include <libavformat/avformat.h>
// #include <libavutil/avutil.h>
// #include <libavutil/buffer.h>
// #include <libavutil/hwcontext.h>
// #include <libswscale/swscale.h>
// #include <libavfilter/avfilter.h>

#include "../include/video_out_streamer.h"
#include "egl_common.h"

#include <vulkan/vulkan.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>

struct __QO_EGLVersion
{
    EGLint major;
    EGLint minor;
};
typedef struct __QO_EGLVersion _QO_EGLVersion;
_QO_EGLVersion __egl_version;

// Maybe we can share it between processes via DMA-BUF in linux or D3DShareHandle in windows
// But I'm not intended to do that in version 1.x
struct _QO_EGLRenderingEnv
{
    qo_ref_count_t  ref_count;
    EGLDisplay      egl_display;
    EGLContext      egl_context;
    EGLConfig       egl_config;
};

qo_stat_t
__qo_hw_rendering_env_init(
    QO_RenderingDevice *    p_device ,
    QO_Colorspace           colorspace ,
    QO_RenderingEnv *       p_env
) {
    if (colorspace == QO_COLORSPACE_GRAY8)
    {
        QO_ERRPRINTF("Gray8 is not supported\n");
        return QO_NOT_SUPPORTED;
    }

    if (!__qo_init_egl_ext_functions())
        return QO_NOT_SUPPORTED;

    EGLint display_attributes[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE ,
        EGL_NONE, //< to be set
        EGL_NONE
    };
    switch (p_device->__backend)
    {
        case QO_RENDERING_EGL_VULKAN:
            display_attributes[1] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            break;
        case QO_RENDERING_EGL_D3D11:
            display_attributes[1] = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
            break;
        default:
            return QO_INVALID_ARG;
    }
    

    EGLDisplay egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE , EGL_DEFAULT_DISPLAY , display_attributes);
    if (egl_display == EGL_NO_DISPLAY) 
    {
        QO_ERRPRINTF("Failed to get egl display\n");
        return QO_NOT_SUPPORTED;
    }

    EGLint major , minor;
    if (!eglInitialize(egl_display , &__egl_version.major , &__egl_version.minor))
    {
        QO_ERRPRINTF("Failed to initialize egl\n");
        return QO_NOT_SUPPORTED;
    }

    const EGLint config_attributes[] = {
        EGL_SURFACE_TYPE ,      EGL_PBUFFER_BIT ,
        EGL_RED_SIZE ,          8 ,
        EGL_GREEN_SIZE ,        8 ,
        EGL_BLUE_SIZE ,         8 ,
        EGL_ALPHA_SIZE ,        (colorspace == QO_COLORSPACE_RGBA ? 8 : 0) ,
        EGL_RENDERABLE_TYPE , EGL_OPENGL_ES3_BIT_KHR ,
        EGL_NONE
    };

    EGLint num_config;
    EGLConfig egl_config;
    if (!eglChooseConfig(egl_display , config_attributes , &egl_config , 1, &num_config) || num_config == 0)
    {
        eglTerminate(egl_display);
        QO_ERRPRINTF("Failed to choose egl config\n");
        return QO_NOT_SUPPORTED;
    } 

    p_env->egl_display = egl_display;
    p_env->egl_config  = egl_config;
    return QO_OK;
}

void
__qo_hw_rendering_env_unref(
    QO_RenderingEnv *    p_env
){
    if (--p_env->ref_count)
        return;
    eglDestroyContext(p_env->egl_display , p_env->egl_context);
    eglTerminate(p_env->egl_display);
    p_env->egl_display = EGL_NO_DISPLAY;
    free(p_env);
}

struct __QO_CanvasEGLContext
{
    EGLSurface egl_surface;
};
typedef struct __QO_CanvasEGLContext _QO_CanvasEGLContext;

_QO_CanvasEGLContext *
__qo_canvas_get_egl_context(
    QO_Canvas * canvas
)   {
    return (_QO_CanvasEGLContext *)canvas->extra_context;
}

qo_stat_t
__qo_egl_canvas_init(
    QO_Canvas *         canvas ,
    QO_RenderingEnv *   p_env ,
    qo_uint32_t         width ,
    qo_uint32_t         height ,
    QO_Color            default_background_color
) {

    const EGLint pbuffer_attributes[] = {
        EGL_WIDTH , width ,
        EGL_HEIGHT , height ,
        EGL_NONE
    };

    EGLSurface egl_surface = eglCreatePbufferSurface(p_env->egl_display , p_env->egl_config , pbuffer_attributes);
    if (egl_surface == EGL_NO_SURFACE)
    {
        QO_ERRPRINTF("Failed to create egl surface\n");
        return QO_NOT_SUPPORTED;
    }

    _QO_CanvasEGLContext * canvas_egl_context = __qo_canvas_get_egl_context(canvas);
    canvas_egl_context->egl_surface = egl_surface;
    return QO_OK;
}

void
__qo_egl_canvas_deinit(
    QO_Canvas * canvas
) {
    if (canvas)
    {
        _QO_CanvasEGLContext * canvas_egl_context = __qo_canvas_get_egl_context(canvas);
        eglDestroySurface(canvas->p_bound_env->egl_display , canvas_egl_context->egl_surface);
        canvas_egl_context->egl_surface = EGL_NO_SURFACE;
        __qo_hw_rendering_env_unref(canvas->p_bound_env);
    }
}

void
__qo_egl_canvas_auto_switch(
    QO_Canvas * canvas
) {
    _QO_CanvasEGLContext * canvas_egl_context = __qo_canvas_get_egl_context(canvas);
    if (canvas_egl_context->egl_surface != eglGetCurrentSurface(EGL_DRAW))
        eglMakeCurrent(
            canvas->p_bound_env->egl_display , 
            canvas_egl_context->egl_surface , 
            canvas_egl_context->egl_surface , 
            canvas->p_bound_env->egl_context
        );
}

// Maybe we can make a "unsafe" version of this function, which will not switch
// EGL context, and the caller should make sure the context is correct.
qo_stat_t
__qo_egl_canvas_draw_rectangle(
    QO_Canvas * canvas ,
    qo_uint32_t x ,
    qo_uint32_t y ,
    qo_uint32_t width ,
    qo_uint32_t height ,
    QO_Color color
) {
    __qo_egl_canvas_auto_switch(canvas);

}