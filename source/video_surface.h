#pragma once

#define __QOR_VIDEO_SURFACE_SRC__

#include "output_surface.h"

struct __VideoSurface
{
    _IOutputSurface     base;
    VkExtent2D          extent;
    VkSurfaceFormatKHR  surface_format;

};
