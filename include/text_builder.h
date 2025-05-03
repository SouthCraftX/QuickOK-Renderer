#pragma once
#define __QOR_TEXT_BUILDER_H__

#if defined (__cplusplus)
extern "C" {
#endif // __cplusplus

#include "../../QuickOK-Zero/include/qozero.h"
#include "font_engine.h"

typedef enum 
{
    QOR_TEXT_INST_NONE = 0,
    QOR_TEXT_INST_NEWLINE,
    QOR_TEXT_INST_CSTR,
    QOR_TEXT_INST_VSTR,
    QOR_TEXT_INST_CHAR,
    QOR_TEXT_INST_HEX64,
    QOR_TEXT_INST_HEX32,
    QOR_TEXT_INST_HEXT, // trim front zeros
    QOR_TEXT_INST_OCT64,
    QOR_TEXT_INST_OCT32,
    QOR_TEXT_INST_OCTT, // trim front zeros
    QOR_TEXT_INST_BIN64,
    QOR_TEXT_INST_BIN32,
    QOR_TEXT_INST_BINT, // trim front zeros
    QOR_TEXT_INST_UDEC64, // All UDEC are trimmed
    QOR_TEXT_INST_UDEC32,
    QOR_TEXT_INST_SDEC64,
    QOR_TEXT_INST_SDEC32,
    QOR_TEXT_INST_FP32,
    QOR_TEXT_INST_FP64
} QOR_TextBuildingInstruction;

struct _QOR_TextInstructionEntry
{
    QOR_TextBuildingInstruction   inst;
    qo_pointer_t                  resource;
};
typedef struct _QOR_TextInstructionEntry QOR_TextInstructionEntry;

struct _QOR_TextBuilderPipeline;
typedef struct _QOR_TextBuilderPipeline QOR_TextBuilderPipeline;

struct _QOR_GraphemeCollector;
typedef struct _QOR_GraphemeCollector QOR_GraphemeCollector;

struct _QOR_GraphemesView
{
    qo_ccstring_t * utf_characters;
    qo_size_t                   count;
};
typedef struct _QOR_GraphemesView QOR_GraphemesView;

struct _QOR_TextBuilder;
typedef struct _QOR_TextBuilder QOR_TextBuilder;

QOR_TextBuilderPipeline *
qor_text_builder_pipeline_new();

void
qor_text_builder_pipeline_unref(
    QOR_TextBuilderPipeline *   pipeline
);

qo_stat_t
qor_text_builder_pipeline_add(
    QOR_TextBuilderPipeline *   pipeline ,
    QOR_TextInstructionEntry *   entry ,
    qo_size_t                   count
) QO_NONNULL(1 , 2);

QOR_TextBuilder *
qor_text_builder_new(
    QOR_TextBuilderPipeline *   pipeline ,
    QO_FontEngine *             font_engine
) QO_NONNULL(1 , 2);

QOR_GraphemeCollector *
qor_grapheme_collector_new();

void
qor_grapheme_collector_unref(
    QOR_GraphemeCollector *         collector
);

qo_size_t
qor_grapheme_collector_sample(
    QOR_GraphemeCollector *         collector ,
    qo_ccstring_t               text
) QO_NONNULL(1 , 2);

#if defined (__cplusplus)
}
#endif // __cplusplus