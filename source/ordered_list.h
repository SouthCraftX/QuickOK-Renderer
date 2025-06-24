#pragma once
#define __QOR_ORDERED_LIST_SRC__

#include "rendering_env.h"

typedef 
    qo_pointer_t
    (* ol_item_copy_f)(
        qo_cpointer_t   item
    );

typedef
    void
    (* ol_item_destroy_f)(
        qo_pointer_t    item
    );

struct __OrderedListAuxiliary
{
    ol_item_copy_f      item_copy_func;
    ol_item_destroy_f   item_destroy_func;
};
typedef struct __OrderedListAuxiliary _OrderedListAuxiliary;

struct __OrderedList
{
    qo_pointer_t *         items;
    qo_size_t              item_count;
    qo_size_t              capacity;
    _OrderedListAuxiliary  auxiliary;
};
typedef struct __OrderedList _OrderedList;

qo_stat_t
ordered_list_init(
    _OrderedList *              self,
    qo_size_t                   item_size ,
    qo_size_t                   capacity ,
    _OrderedListAuxiliary *     p_auxiliary
);

void
ordered_list_destroy(
    _OrderedList * self
);

qo_bool_t
ordered_list_add(
    _OrderedList * self,
    qo_cpointer_t item
);

qo_pointer_t
ordered_list_get(
    _OrderedList const * self,
    qo_size_t           index
);

qo_size_t
ordered_list_get_count(
    _OrderedList const * self
);
