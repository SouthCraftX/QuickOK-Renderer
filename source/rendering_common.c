#include "../include/renderer.h"
#include "../../QuickOK-Concurrency/include/thread.h"
#include "egl_common.h"
#include <stddef.h>

QO_RenderingEnv * __qo_g_software_rendering_env = NULL;
QO_RenderingEnv * __qo_g_hardware_rendering_env = NULL;

QO_THREAD_LOCAL 
QO_RenderingEnv *  __qo_tls_software_rendering_env = NULL;

QO_THREAD_LOCAL 
QO_RenderingEnv *  __qo_tls_hardware_rendering_env = NULL;

struct _QO_RenderingEnv 
{
    qo_ref_count_t          reference_count;
    QO_RenderingBackends    backend;
    QO_Colorspace           colorspace;
    QO_Thread *             owner_thread;  //< NULL if shared between threads
    QO_VLA                  extra_context;
};

QO_RenderingBackends
qo_rendering_env_get_type(
    QO_RenderingEnv *   p_env
) {
    return p_env->backend;
}

extern
qo_stat_t
__qo_hw_rendering_env_init(
    QO_RenderingBackends    backend ,
    QO_Colorspace           colorspace ,
    qo_ccstring_t           hw_device_name ,
    QO_RenderingEnv **      pp_env
);

qo_stat_t
__qo_rendering_env_get_common(
    QO_RenderingBackends    backend ,
    QO_Colorspace           colorspace ,
    qo_ccstring_t           hw_device_name ,
    QO_RenderingEnv **      pp_returned_env ,
    QO_RenderingEnv *       p_hw_rendering_env ,
    // QO_RenderingEnv *       p_sw_rendering_env ,
    QO_RenderingEnv **      pp_out_rendering_env
) {
    if (backend == QO_RENDERING_SOFTWARE)
        return QO_NOT_IMPLEMENTED;

    if(p_hw_rendering_env == NULL) 
    {
        qo_stat_t stat = __qo_hw_rendering_env_init(backend , colorspace , hw_device_name , pp_out_rendering_env);
        if (stat != QO_OK)
            return stat;
    }
    else
    {
        qo_bool_t match = (backend == p_hw_rendering_env->backend) && 
                          (colorspace == p_hw_rendering_env->colorspace);
        if (!match)
            return QO_MISMATCH;
    }
    *pp_returned_env = *pp_out_rendering_env;
    return QO_OK;
}

qo_stat_t
__qo_tls_rendering_env_get(
    QO_RenderingBackends    backend ,
    QO_Colorspace           colorspace ,
    qo_ccstring_t           hw_device_name ,
    QO_RenderingEnv **      pp_returned_env
) {
    return __qo_rendering_env_get_common(
        backend ,
        colorspace ,
        hw_device_name ,
        pp_returned_env ,
        __qo_tls_hardware_rendering_env ,
        // __qo_tls_software_rendering_env ,
        &__qo_tls_hardware_rendering_env
    );
}

qo_stat_t
__qo_g_rendering_env_get(
    QO_RenderingBackends    backend,
    QO_Colorspace           colorspace,
    qo_ccstring_t           hw_device_name,
    QO_RenderingEnv **      pp_returned_env
) {
    return __qo_rendering_env_get_common(
        backend ,
        colorspace ,
        hw_device_name ,
        pp_returned_env ,
        __qo_g_hardware_rendering_env ,
        // __qo_g_software_rendering_env ,
        &__qo_g_hardware_rendering_env 
    );
}

qo_stat_t
qo_rendering_env_get(
    QO_RenderingBackends    backend , 
    QO_Colorspace           colorspace , 
    qo_ccstring_t           hw_device_name , 
    QO_RenderingEnv **      pp_env , 
    qo_bool_t               shared_between_threads
) {
    return  shared_between_threads ? 
            __qo_tls_rendering_env_get(backend , colorspace , hw_device_name , pp_env) :
            __qo_g_rendering_env_get(backend , colorspace , hw_device_name , pp_env);
}

