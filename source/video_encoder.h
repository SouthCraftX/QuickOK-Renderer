#pragma once
#define __QO_VIDEO_ENCODER_SRC__

#include "../include/video_out_streamer.h"

#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/buffer.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>


struct _QO_VideoEncoder
{
    qo_ref_count_t  reference_count;

    void
    (* destructor)(
        QO_VideoEncoder *   self
    );

    qo_stat_t
    (* write_frame)(
        QO_VideoEncoder *   self ,
    );

    // Common context for all encoders (HW and SW)
    QO_Canvas * p_canvas;
    AVFrame *   frame;

    QO_VLA      extra_context;
};

struct __QO_HWVideoEncoderContext
{
    AVBufferRef *           hw_device_ctx;
    AVHWFramesContext *     hw_frames_ctx;
};
typedef struct __QO_HWVideoEncoderContext  _QO_HWVideoEncoderContext;

typedef qo_stat_t(* __qo_video_encoder_newer_t)(
    QO_VideoEncoder **  encoder ,
    qo_ccstring_t       codec_name ,
    QO_VideoInfo *      p_video_info ,
    QO_Canvas *         p_canvas 
);

extern
qo_stat_t
__qo_video_encoder_d3d11_new(
    QO_VideoEncoder **  encoder ,
    qo_ccstring_t       codec_name ,
    QO_VideoInfo *      p_video_info ,
    QO_Canvas *         p_canvas
);

extern
qo_stat_t
__qo_video_encoder_vulkan_new(
    QO_VideoEncoder **  encoder ,
    qo_ccstring_t       codec_name ,
    QO_VideoInfo *      p_video_info ,
    QO_Canvas *         p_canvas
);

qo_bool_t
__qo_is_codec_hw(
    AVCodec const * codec
) {
    for (int i = 0 ; ; i++) 
    {
        AVCodecHWConfig const * hw_config = avcodec_get_hw_config(codec, i);
        if (!hw_config)
            break;
        if (hw_config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
            return qo_true;
    }
    return qo_false;
}

qo_stat_t
qo_video_encoder_new(
    QO_VideoEncoder **  encoder ,
    qo_ccstring_t       codec_name ,
    QO_VideoInfo *      p_video_info ,
    QO_Canvas *         p_canvas 
) {
    AVCodec const * codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) 
        return QO_NOT_FOUND;
    
    if ((__qo_is_codec_hw(codec) ^ qo_canvas_is_backend_hardware(p_canvas)) == 0) 
        return QO_NOT_SUPPORTED; // Only when they are all hardware or all software
    
    

}