#pragma once
#include <libavutil/pixfmt.h>
#define __QOR_VIDEO_OUT_STREAMER_H__

#include "canvas.h"
#include <libavformat/avformat.h>

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

struct _QO_VideoEncoder;
typedef struct _QO_VideoEncoder QO_VideoEncoder;

struct _QO_OutVideoStream;
typedef struct _QO_OutVideoStream QO_OutVideoStream;

struct _QO_VideoInfo
{
    qo_uint32_t         width;
    qo_uint32_t         height;
    QO_DivisionInt32    fps;
    AVPixelFormat       pixel_format;
};
typedef struct _QO_VideoInfo QO_VideoInfo;

/// @brief Create a new video encoder.
/// @param pp_encoder Pointer to the pointer to the new encoder.
/// @param codec_name The name of the codec.
/// @param p_info Pointer to the video info.
/// @param p_hw_rendering_info Pointer to the hardware rendering info. Set it NULL
///        if your canvas does not support hardware rendering. e.g. "ANGLE:D3D11"
///        or "ANGLE:Vulkan"
qo_stat_t
qo_video_encoder_new(
    QO_VideoEncoder **  pp_encoder ,
    qo_ccstring_t       codec_name ,
    QO_VideoInfo *      p_info ,
    QO_Canvas *         p_canvas
);


/// @brief Create a new video stream.
/// @param pp_stream Pointer to the pointer to the new stream.
/// @param video_path The path to the video file.
/// @param container_name The name of the container. Set it to NULL to 
///         automatically detect the container by the file extension.
/// @param p_thread_pool Pointer to the thread pool. 
qo_stat_t
qo_out_video_stream_new(
    QO_OutVideoStream **   pp_stream ,
    qo_ccstring_t           video_path ,
    qo_ccstring_t           container_name ,
    qo_pointer_t            p_thread_pool ,
    qo_flag32_t             flags
) QO_NONNULL(1 , 2 , 4);

/// @brief Flush the stream immediately.
/// @param p_stream Pointer to the stream.
qo_stat_t
qo_out_video_stream_flush(
    QO_OutVideoStream *    p_stream
) QO_NONNULL(1);

/// @brief Bind the encoder to a stream.
/// @param p_encoder Pointer to the encoder.
/// @param p_stream Pointer to the stream.
qo_stat_t
qo_video_encoder_bind_stream(
    QO_VideoEncoder *      p_encoder ,
    QO_OutVideoStream *    p_stream
) QO_NONNULL(1 , 2);

/// @brief Set the QP range and the QP value.
/// @param p_encoder Pointer to the encoder.
/// @param min_qp The minimum QP value. Set it -1 to leave it unchanged.
/// @param max_qp The maximum QP value. Set it -1 to leave it unchanged.
/// @param qp The QP value. Set it -1 to leave it unchanged.
/// @return The status of the operation.
/// @retval QO_OK The operation was successful.
/// @retval QO_INVALID_ARG Specified qp is out of range.
qo_stat_t
qo_video_encoder_set_qp(
    QO_VideoEncoder *  p_encoder ,
    qo_int8_t           min_qp ,
    qo_int8_t           max_qp ,
    qo_int8_t           qp
) QO_NONNULL(1);

/// @brief Set the preset.
/// @param p_encoder Pointer to the encoder.
/// @param preset The preset to set. See the FFmpeg documentation for more information.
/// @return The status of the operation.
/// @retval QO_OK The operation was successful.
/// @retval QO_INVALID_ARG Specified preset is not supported.
qo_stat_t
qo_video_encoder_set_preset(
    QO_VideoEncoder *  p_encoder ,
    qo_ccstring_t       preset
) QO_NONNULL(1 , 2);

/// @brief Set the bitrate.
/// @param p_encoder Pointer to the encoder.
/// @param max_bitrate The maximum bitrate. Set it -1 to leave it unchanged.
/// @param min_bitrate The minimum bitrate. Set it -1 to leave it unchanged.
/// @param bitrate The bitrate. Set it -1 to leave it unchanged.
/// @return The status of the operation.
/// @retval QO_OK The operation was successful.
qo_stat_t
qo_video_encoder_set_bitrate(
    QO_VideoEncoder *  p_encoder ,
    qo_size_t           max_bitrate ,
    qo_size_t           min_bitrate ,
    qo_size_t           bitrate 
) QO_NONNULL(1);

/// @brief Set the CRF.
/// @param p_encoder Pointer to the encoder.
/// @param crf_max The maximum CRF value. Set it -1 to leave it unchanged.
/// @param crf The CRF value. Set it -1 to leave it unchanged.
/// @return The status of the operation.
/// @retval QO_OK The operation was successful.
/// @retval QO_INVALID_ARG Specified crf is out of range.
qo_stat_t
qo_video_encoder_set_crf(
    QO_VideoEncoder *  p_encoder ,
    qo_int8_t           crf_max ,
    qo_int8_t           crf
) QO_NONNULL(1);

/// @brief Submit a frame from the canvas previously bound to the encoder.
qo_stat_t
qo_video_encoder_submit(
    QO_VideoEncoder *  p_encoder
) QO_NONNULL(1);

/// @brief Decrement the reference count of the encoder.
/// @param p_encoder Pointer to the encoder. NULL is allowed.
qo_stat_t
qo_video_encoder_unref(
    QO_VideoEncoder * p_encoder
);

#if defined(__cplusplus)
}
#endif // __cplusplus