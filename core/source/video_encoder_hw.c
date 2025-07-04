#include "video_encoder.h"
#include <libavutil/frame.h>
#include <libavutil/hwcontext.h>

extern
qo_stat_t
__qo_setup_hw_device_context(
    _QO_HWVideoEncoderContext *  p_hw_ctx 
);

qo_stat_t
__qo_video_encoder_generic_hw_new(
    QO_VideoEncoder **  pp_encoder ,
    AVCodec const *     codec ,
    QO_VideoInfo *      p_info ,
    QO_Canvas *         p_canvas  
) {
    qo_stat_t           stat = QO_OK;
    int                 ret = 0; // FFMpeg APIs return int
    QO_VideoEncoder *   p_encoder = NULL;
    AVFrame *           frame_in = NULL;
    _QO_HWVideoEncoderContext hw_ctx = {NULL , NULL};

    p_encoder = malloc(sizeof(QO_VideoEncoder) + sizeof(_QO_HWVideoEncoderContext)); // TODO: Need VLA size
    if (!p_encoder) 
    {
        stat = QO_OUT_OF_MEMORY;
        goto J_FAIL;
    }
    
    enum AVHWDeviceType hw_device_types[] = {
        0 ,
        AV_HWDEVICE_TYPE_D3D11VA , //< QO_CANVAS_D3D11
        AV_HWDEVICE_TYPE_VULKAN //< QO_CANVAS_VULKAN
    };
    


    frame_in = av_frame_alloc();
    if (!frame_in)
    {
        stat = QO_OUT_OF_MEMORY;
        goto J_FAIL;
    }

    frame_in->height = p_info->height;
    frame_in->width  = p_info->width;
    frame_in->format = p_info->pixel_format;
    
    
J_FAIL:
    if (p_encoder) 
        mi_free(p_encoder);
    if (frame_in)
        av_frame_free(&frame_in);
    return stat;
}