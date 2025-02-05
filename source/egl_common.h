#pragma once
#define __QO_EGL_COMMON_SRC__

#include "../include/renderer.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>

typedef EGLDisplay (* _fn_eglGetPlatformDisplayEXT)(
        EGLenum , 
        void * , 
        EGLint const *
);

typedef EGLBoolean (* _fn_eglQueryDeviceAttribEXT)(
        EGLDisplay ,
        EGLDeviceEXT ,
        EGLint ,
        EGLAttrib *
);

typedef char const * (* _fn_eglQueryDeviceStringEXT)(
        EGLDisplay ,
        EGLDeviceEXT ,
        EGLint
);

extern
_fn_eglGetPlatformDisplayEXT eglGetPlatformDisplayEXT;

extern
_fn_eglQueryDeviceAttribEXT eglQueryDeviceAttribEXT;

extern
_fn_eglQueryDeviceStringEXT eglQueryDeviceStringEXT;

QO_GLOBAL_UNIQUE
qo_bool_t
__qo_init_egl_ext_functions()
{
    static qo_bool_t ret , called = qo_false;
    if (called)
        return ret;

    eglGetPlatformDisplayEXT = (_fn_eglGetPlatformDisplayEXT)eglGetProcAddress("eglGetPlatformDisplayEXT");
    eglQueryDeviceAttribEXT = (_fn_eglQueryDeviceAttribEXT)eglGetProcAddress("eglQueryDeviceAttribEXT");
    eglQueryDeviceStringEXT = (_fn_eglQueryDeviceStringEXT)eglGetProcAddress("eglQueryDeviceStringEXT");

    ret = eglQueryDeviceAttribEXT && eglGetPlatformDisplayEXT && eglQueryDeviceStringEXT;

    // TODO: Replace with proper logging
    if (!eglGetPlatformDisplayEXT)
        QO_ERRPRINTF("Extension eglGetPlatformDisplayEXT not found");
    if (!eglQueryDeviceAttribEXT)
        QO_ERRPRINTF("Extension eglQueryDeviceAttribEXT not found");
    if (!eglQueryDeviceStringEXT)
        QO_ERRPRINTF("Extension eglQueryDeviceStringEXT not found");

    called = qo_true;
    return ret;
}

