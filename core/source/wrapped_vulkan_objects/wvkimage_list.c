#include "wrapped_vulkan_image_list.h"
#include "wrapped_vulkan_image.h"
#include <mimalloc.h>

void
wvkimage_list_init(
    _WVkImageList * self
) {
    memset(self, 0, sizeof(_WVkImageList));
}

qo_bool_t
wvkimage_list_is_empty(
    _WVkImageList * self
) {
    return self->count == 0;
}

qo_bool_t
wvkimage_list_push_back(
    _WVkImageList * self ,
    _WVkImage *     image
) {
    _WVkImageListNode * node = mi_malloc_tp(_WVkImageListNode);
    if (!node)
    {
        return qo_false;
    }

    node->image = image;
    node->next = NULL;
    node->prev = self->tail;
    if (wvkimage_list_is_empty(self))
    {
        self->head = node;
        self->tail = node;
    }
    else {
        self->tail->next = node;
        self->tail = node;
    }
    self->count++;
    return qo_true;
}

_WVkImage *
wvkimage_list_pop_front(
    _WVkImageList * self
) {
    if (wvkimage_list_is_empty(self))
    {
        return NULL;
    }
    _WVkImageListNode * node = self->head;
    self->head = node->next;
    if (self->head)
    {
        self->head->prev = NULL;
    }
    else {
        self->tail = NULL;
    }
    self->count--;
    mi_free(node);
    return node->image;
}

qo_size_t
wvkimage_list_clear(
    _WVkImageList * self
) {
    qo_size_t count = self->count;
    while (!wvkimage_list_is_empty(self))
    {
        _WVkImage * image = wvkimage_list_pop_front(self);
        wvkimage_unref(image);
    }
    return count;
}

void
wvkimage_list_destory(
    _WVkImageList * self
) {
    wvkimage_list_clear(self);
    memset(self, 0, sizeof(_WVkImageList));
}



