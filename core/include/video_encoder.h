#pragma once
#define __QOR_VIDEO_ENCODER_H__

#include "renderer.h"

typedef enum
{
    __QOR_VIDEO_NULL = 0 ,
    __QOR_VIDEO_WIDTH,
    __QOR_VIDEO_HEIGHT,
    __QOR_VIDEO_FPS,      // Variable FPS is not supported 
    __QOR_VIDEO_CRF,
    __QOR_VIDEO_CRF_MAX,
    __QOR_VIDEO_AVERAGE_BITRATE,
    __QOR_VIDEO_MAX_BITRATE,
    __QOR_VIDEO_MIN_BITRATE,
    __QOR_VIDEO_AVERAGE_QP,
    __QOR_VIDEO_MAX_QP,
    __QOR_VIDEO_MIN_QP,
    __QOR_VIDEO_PRESET,
    __QOR_VIDEO_PROFILE,
    __QOR_VIDEO_TUNE,
    __QOR_VIDEO_GOP_SIZE,
    __QOR_VIDEO_MAX_B_FRAMES,
    __QOR_VIDEO_REF_P_COUNT,
    __QOR_VIDEO_MOTION_EVALUATION,
    __QOR_VIDEO_MOTION_EVALUATION_RANGE,
    __QOR_VIDEO_DEBLOCK,
    __QOR_VIDEO_NOISE_REDUCTION,
    __QOR_VIDEO_KEYINT_MIN,
    __QOR_VIDEO_AQ_MODE,
    __QOR_VIDEO_CABAC,
    __QOR_VIDEO_SCENECUT,
    __QOR_VIDEO_INTRA,
    __QOR_VIDEO_REF
} __QOR_VideoArgmentType;

struct _QOR_VideoArgument
{
    __QOR_VideoArgmentType  type;
    union
    {
        qo_bool_t           b;
        qo_uint8_t          u8;
        qo_int8_t           i8;
        qo_int16_t          i16;
        qo_uint16_t         u16;
        qo_uint32_t         u32;
        qo_int32_t          i32;
        qo_uint64_t         u64;
        qo_int64_t          i64;
        qo_fp32_t           fp32;
        qo_fp64_t           fp64;
        qo_ccstring_t       str;
        qo_pointer_t        ptr;
        QO_DivisionInt32    div32;
    } value;
};
typedef struct _QOR_VideoArgument QOR_VideoArgument;

#define QOR_VIDEO_WIDTH(v) {__QOR_VIDEO_WIDTH, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_HEIGHT(v) {__QOR_VIDEO_HEIGHT, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_FPS(v) {__QOR_VIDEO_FPS, {.fp32 = (qo_fp32_t)(v)}}
#define QOR_VIDEO_CRF(r , d) {__QOR_VIDEO_CRF, {.div32 = (QO_DivisionInt32){(qo_int32_t)(r), (qo_int32_t)(d)}}}
#define QOR_VIDEO_CRF_MAX(v) {__QOR_VIDEO_CRF_MAX, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_AVERAGE_BITRATE(v) {__QOR_VIDEO_AVERAGE_BITRATE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_MAX_BITRATE(v) {__QOR_VIDEO_MAX_BITRATE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_MIN_BITRATE(v) {__QOR_VIDEO_MIN_BITRATE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_PRESET(v) {__QOR_VIDEO_PRESET, {.str = (qo_ccstring_t)(v)}}
#define QOR_VIDEO_PROFILE(v) {__QOR_VIDEO_PROFILE, {.str = (qo_ccstring_t)(v)}}
#define QOR_VIDEO_TUNE(v) {__QOR_VIDEO_TUNE, {.str = (qo_ccstring_t)(v)}}
#define QOR_VIDEO_GOP_SIZE(v) {__QOR_VIDEO_GOP_SIZE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_MAX_B_FRAMES(v) {__QOR_VIDEO_MAX_B_FRAMES, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_REF_P_COUNT(v) {__QOR_VIDEO_REF_P_COUNT, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_MOTION_EVALUATION(v) {__QOR_VIDEO_MOTION_EVALUATION, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_MOTION_EVALUATION_RANGE(v) {__QOR_VIDEO_MOTION_EVALUATION_RANGE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_DEBLOCK(v) {__QOR_VIDEO_DEBLOCK, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_NOISE_REDUCTION(v) {__QOR_VIDEO_NOISE_REDUCTION, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_KEYINT_MIN(v) {__QOR_VIDEO_KEYINT_MIN, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_AQ_MODE(v) {__QOR_VIDEO_AQ_MODE, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_CABAC(v) {__QOR_VIDEO_CABAC, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_SCENECUT(v) {__QOR_VIDEO_SCENECUT, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_INTRA(v) {__QOR_VIDEO_INTRA, {.i32 = (qo_int32_t)(v)}}
#define QOR_VIDEO_REF(v) {__QOR_VIDEO_REF, {.i32 = (qo_int32_t)(v)}}


