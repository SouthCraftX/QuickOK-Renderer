#pragma once
#include <mimalloc.h>
#define __QOR_VECTOR_H__

#include "../rendering_env.h"

#define VECTOR_INIT_COUNT  8

struct __Vector
{
    qo_pointer_t  data;
    qo_size_t     size;
    qo_size_t     capacity;
    qo_size_t     element_size;
};
typedef struct __Vector _Vector;

struct __VectorIterator
{
    _Vector * vector;
    qo_size_t index;
};
typedef struct __VectorIterator _VectorIterator;

QO_NODISCARD QO_NONNULL(1)
qo_bool_t
vector_init(
    _Vector *  self ,
    qo_size_t  element_size
) {
    QO_ASSERT(self);
    memset(self , 0 , sizeof(_Vector));
    self->element_size = element_size;
    self->capacity = VECTOR_INIT_COUNT;
    qo_pointer_t  data = mi_malloc(self->capacity * self->element_size);
    if (data == NULL)
    {
        return qo_false;
    }
    self->data = data;
    return qo_true;
}

void
vector_destroy(
    _Vector * self
) {
    mi_free(self->data);
    memset(self , 0 , sizeof(_Vector));
}

QO_NODISCARD QO_NONNULL(1)
qo_bool_t
vector_is_empty(
    _Vector const * self
) {
    QO_ASSERT(self);
    return self->size == 0;
}

QO_NODISCARD QO_NONNULL(1)
qo_bool_t
vector_reserve(
    _Vector * self ,
    qo_size_t count
) {
    QO_ASSERT(self);
    if (self->capacity >= count)
    {
        return qo_true;
    }
    qo_pointer_t new_data = mi_reallocn(self->data , self->element_size , count);
    if (!new_data)
    {
        return qo_false;
    }
    self->data = new_data;
    self->capacity = count;
    return qo_true;
}

QO_NODISCARD QO_NONNULL(1)
qo_pointer_t
vector_at(
    _Vector const * self ,
    qo_size_t       index
) {
    QO_ASSERT(index < self->size);
    return (qo_pointer_t)((qo_uint8_t *)self->data + index * self->element_size);
}

QO_NODISCARD QO_NONNULL(1)
qo_pointer_t
vector_back(
    _Vector const * self
) {
    QO_ASSERT(self);
    return vector_is_empty(self) ? NULL : vector_at(self , 0);
}

QO_NODISCARD QO_NONNULL(1 , 2)
qo_bool_t
vector_push_back(
    _Vector *       self ,
    qo_cpointer_t   object
) {
    QO_ASSERT(self && object);
    if (self->size >= self->capacity)
    {
        qo_size_t new_capacity = self->capacity * 2;
        if (!vector_reserve(self , new_capacity))
        {
            return qo_false;
        }
    }
    qo_pointer_t dest = vector_at(self , self->size);
    memcpy(dest , object , self->element_size);
    self->size++;
    return qo_true;
}

void
vector_clear(
    _Vector *   self
) {
    QO_ASSERT(self);
    self->size = 0;
}

QO_NODISCARD QO_NONNULL(1)
qo_size_t
vector_get_count(
    _Vector const * self
) {
    QO_ASSERT(self);
    return self->size;
}

QO_NODISCARD QO_NONNULL(1)
qo_pointer_t
vector_get_data(
    _Vector const * self
) {
    QO_ASSERT(self);
    return self->data;
}

QO_NODISCARD QO_NONNULL(1)
qo_size_t
vector_get_capacity(
    _Vector const * self
) {
    QO_ASSERT(self);
    return self->capacity;
}

QO_NODISCARD QO_NONNULL(1)
_VectorIterator
vector_iterate_begin(
    _Vector * self
) {
    return (_VectorIterator) {
        self , 0
    };
}

QO_NODISCARD
_VectorIterator
vector_iterate_end(
    _Vector * self
) {
    return (_VectorIterator) {
        self , self ? self->size : 0
    };
}

QO_NONNULL(1)
void
vector_iterator_next(
    _VectorIterator *   self
) {
    if (self->vector && self->index < self->vector->size)
    {
        self->index++;
    }
}

QO_NONNULL(1)
void
vector_iterator_prev(
    _VectorIterator *   self
) {
    if (self->index > 0)
    {
        self->index--;
    }
}

QO_NODISCARD QO_NONNULL(1)
qo_pointer_t
vector_iterator_get(
    _VectorIterator const * self
) {
    return (self->vector && self->index < self->vector->size) ?
           vector_at(self->vector , self->index) : NULL;
}

QO_NONNULL(1)
void
vector_iterator_set(
    _VectorIterator *   self ,
    qo_cpointer_t       object
) {
    if (self->vector && self->index < self->vector->size)
    {
        memcpy(vector_at(self->vector , self->index) , object , self->vector->element_size);
    }
}

QO_NODISCARD QO_NONNULL(1 , 2)
qo_bool_t
vector_iterator_equals(
    _VectorIterator const * a ,
    _VectorIterator const * b
) {
    return a->vector == b->vector && a->index == b->index;
}
