#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_OUTPUT_SURFACE_H__

#include "rendering_env.h"

typedef enum
{
    OUTPUT_SURFACE_ACQUIRE_SUCCESS ,
    OUTPUT_SURFACE_ACQUIRE_SUBOPTIMAL ,
    OUTPUT_SURFACE_ACQUIRE_REBUILD_NEEDED ,
    OUTPUT_SURFACE_ACQUIRE_ERROR
} _OutputSurfaceAcquireResult;

typedef enum
{
    OUTPUT_SURFACE_RESIZABLE ,
    OUTPUT_SURFACE_FORMAT_VARIABLE ,
    OUTPUT_SURFACE_TRANSFORMABLE
} _OutputSurfaceAttributes;

struct __IOutputSurface;
typedef struct __IOutputSurface _IOutputSurface;
// Bridge rendering and presenting
struct __IOutputSurface
{
    void
    (* destroy)(
        _IOutputSurface * self
    );

    qo_bool_t
    (* query_attribute)(
        _IOutputSurface *      self ,
        _OutputSurfaceAttributes  attribute
    );

    qo_bool_t
    (* should_close)(
        _IOutputSurface * self ,
        qo_stat_t *          p_reason
    );

    VkExtent2D
    (* get_extent)(
        _IOutputSurface * self
    );

    VkSurfaceFormat2KHR
    (* get_surface_format)(
        _IOutputSurface * self
    );

    void
    (* trigger_rebuild)(
        _IOutputSurface * self
    );

    _OutputSurfaceAcquireResult
    (* acquire_next_image)(
        _IOutputSurface * self ,
        qo_int32_t *         p_image_index ,
        VkSemaphore          image_ready_semaphore
    );

    qo_bool_t
    (* present)(
        _IOutputSurface * self ,
        qo_int32_t           image_index ,
        VkQueue              present_queue ,
        VkSemaphore          render_finished_semaphore
    );

    void
    (* inform_done)(
        _IOutputSurface * self ,
        qo_stat_t            status
    );
};
