#include "ordered_list.h"
#include <mimalloc.h>



_OrderedList *
ordered_list_new(
    qo_size_t               item_size ,
    qo_size_t               capacity ,
    _OrderedListAuxiliary * p_auxiliary
) {
    _OrderedList * self = mi_malloc_tp(_OrderedList);
    if (self == NULL)
    {
        return NULL;
    }

    self->capacity = capacity ? capacity : 8;
    self->items = mi_mallocn_tp(qo_pointer_t , self->capacity);
    self->auxiliary  = *p_auxiliary;
    self->item_count = 0;
    if (self->items == NULL)
    {
        mi_free(self);
        return NULL;
    }
    return self;
}

void
ordered_list_destroy(
    _OrderedList * self
) {
    if (self->auxiliary.item_destroy_func)
    {
        for (qo_size_t i = 0; i < self->item_count; i++)
        {
            self->auxiliary.item_destroy_func(self->items[i]);
        }
    }
    mi_free(self->items);
    mi_free(self);
}

qo_bool_t
ordered_list_add(
    _OrderedList * self ,
    qo_cpointer_t  item
) {
    if (self->item_count == self->capacity)
    {
        qo_size_t  new_capaicty  = self->capacity * 2;
        qo_pointer_t * new_items = mi_reallocn_tp(self->items , qo_pointer_t ,
            new_capaicty);
        if (!new_items)
        {
            return qo_false;
        }
        self->items = new_items;
        self->capacity = new_capaicty;
    }

    // Remove `const`
    qo_pointer_t  item_to_store = (qo_pointer_t) item;
    if (self->auxiliary.item_copy_func)
    {
        item_to_store = self->auxiliary.item_copy_func(item);
    }
    self->items[self->item_count++] = item_to_store;
    return qo_true;
}

qo_size_t
ordered_list_get_count(
    _OrderedList const * self
) {
    return self->item_count;
}
