#pragma once
#include <unicode/utypes.h>
#define __QOR_GRAPHEME_COLLECTOR_SRC__

#include "funnel_hash_table.h"
#include "ordered_list.h"


struct __GraphemeCollector
{
    _FunnelHashTable    hash_table;
    _OrderedList        ordered_list;
    UErrorCode          internal_error;
};
typedef struct __GraphemeCollector _GraphemeCollector;

struct __GraphemeCollectorIterator
{
    _OrderedList const *    list;
    qo_size_t               current_index;
};
typedef struct __GraphemeCollectorIterator _GraphemeCollectorIterator;

qo_bool_t
collect_urf16_graphemes(
    _GraphemeCollector *    self ,
    UChar const *           str ,
    qo_size_t               len
);

qo_bool_t
collect_utf8_graphemes(
    _GraphemeCollector *    self ,
    qo_ccstring_t           str ,
    qo_size_t               len
);

qo_size_t
grapheme_collector_get_count(
    _GraphemeCollector const *    self
);

_GraphemeCollectorIterator
grapheme_collector_iterate(
    _GraphemeCollector const *    self
);

qo_bool_t
grapheme_collector_iterator_next(
    _GraphemeCollectorIterator *    self ,
    UChar **                        p_grapheme ,
    qo_int32_t *                    p_len
);


