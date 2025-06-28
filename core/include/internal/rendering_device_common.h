#pragma once
#define __QO_RENDERING_DEVICE_COMMON_H__

// for debug
#include "../renderer.h"

#if !defined(__QO_RENDERER_H__)
#   error Never include this header directly. Use renderer.h instead.
#endif

struct _QO_RenderingDevice
{
    qo_ccstring_t           __name;
    QO_RenderingBackends    __backend;
    qo_pointer_t            __handle;
};

QO_GLOBAL_UNIQUE
qo_ccstring_t
qo_rendering_device_get_name(
    QO_RenderingDevice * self
) {
    return self->__name;
}

