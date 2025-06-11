#pragma once
#define __QOR_TEXT_BUILDER_H__

#if defined (__cplusplus)
extern "C" {
#endif // __cplusplus

#define QO_ENABLE_EXPERIMENTAL_CXX
#include "../../QuickOK-Zero/include/qozero.h"
#include "font_engine.h"

typedef enum
{
    QOR_TEXT_INST_NONE = 0 ,
    QOR_TEXT_INST_NEWLINE ,
    QOR_TEXT_INST_CSTR ,
    QOR_TEXT_INST_VSTR ,
    QOR_TEXT_INST_CHAR ,
    QOR_TEXT_INST_HEX64 ,
    QOR_TEXT_INST_HEX32 ,
    QOR_TEXT_INST_HEXT , // trim front zeros
    QOR_TEXT_INST_OCT64 ,
    QOR_TEXT_INST_OCT32 ,
    QOR_TEXT_INST_OCTT , // trim front zeros
    QOR_TEXT_INST_BIN64 ,
    QOR_TEXT_INST_BIN32 ,
    QOR_TEXT_INST_BINT , // trim front zeros
    QOR_TEXT_INST_UDEC64 , // All UDEC are trimmed
    QOR_TEXT_INST_UDEC32 ,
    QOR_TEXT_INST_SDEC64 ,
    QOR_TEXT_INST_SDEC32 ,
    QOR_TEXT_INST_FP32 ,
    QOR_TEXT_INST_FP64
} QOR_TextBuildingInstruction;

struct _QOR_TextInstructionEntry
{
    QOR_TextBuildingInstruction  inst;
    qo_pointer_t                 resource;
};
typedef struct _QOR_TextInstructionEntry QOR_TextInstructionEntry;

struct _QOR_TextBuilderPipeline;
typedef struct _QOR_TextBuilderPipeline QOR_TextBuilderPipeline;

struct _QOR_GraphemeCollector;
typedef struct _QOR_GraphemeCollector QOR_GraphemeCollector;

struct _QOR_GraphemeCollectorView
{
    qo_ccstring_t * graphemes;
    qo_size_t       count;
};
typedef struct _QOR_GraphemeCollectorView QOR_GraphemeCollectorView;

struct _QOR_TextBuilder;
typedef struct _QOR_TextBuilder QOR_TextBuilder;
QOR_TextBuilderPipeline *
qor_text_builder_pipeline_new();

void
qor_text_builder_pipeline_unref(
    QOR_TextBuilderPipeline * pipeline
);

qo_stat_t
qor_text_builder_pipeline_add(
    QOR_TextBuilderPipeline *  pipeline ,
    QOR_TextInstructionEntry * entry ,
    qo_size_t                  count
) QO_NONNULL(1 , 2);

QOR_TextBuilder *
qor_text_builder_new(
    QOR_TextBuilderPipeline * pipeline ,
    QO_FontEngine *           font_engine
) QO_NONNULL(1 , 2);

QOR_GraphemeCollector *
qor_grapheme_collector_new();

void
qor_grapheme_collector_delete(
    QOR_GraphemeCollector * collector
);

qo_stat_t
qor_sample_utf8_grapheme(
    QOR_GraphemeCollector * collector ,
    qo_ccstring_t           utf8_text ,
    qo_size_t               utf8_len
) QO_NONNULL(1 , 2);

qo_stat_t
qor_sample_utf16_grapheme(
    QOR_GraphemeCollector * collector ,
    char16_t const *        utf16_text , // TODO: replace with qo_cwstring_t
    qo_size_t               utf16_len
) QO_NONNULL(1 , 2);

qo_stat_t
qor_grapheme_collector_view(
    QOR_GraphemeCollector *     collector ,
    QOR_GraphemeCollectorView * view
) QO_NONNULL(1 , 2);

qo_flag32_t
qor_grapheme_collector_get_uerror(
    QOR_GraphemeCollector * collector
) QO_NONNULL(1);

#if defined (__cplusplus)
}
#endif // __cplusplus
