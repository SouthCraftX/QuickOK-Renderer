#pragma once
#define __QOR_WRAPPED_VULKAN_IMAGE_LIST_SRC__

#include "wrapped_vulkan_image.h"

typedef struct __WVkImageListNode _WVkImageListNode;
struct __WVkImageListNode
{
    _WVkImageListNode * next;
    _WVkImageListNode * prev;
    _WVkImage *         image;
};

struct __WVkImageList
{
    _WVkImageListNode * head;
    _WVkImageListNode * tail;
    qo_size_t           count;
};
typedef struct __WVkImageList _WVkImageList;

_WVkImageList *
wvkimage_list_new();

// Simply copy pointer, not object itself
qo_bool_t
wvkimage_list_push_back(
    _WVkImageList * self,
    _WVkImage *     image
);

void
wvkimage_list_delete(
    _WVkImageList * self
);

qo_bool_t
wvkimage_list_is_empty(
    _WVkImageList * self
);

_WVkImage *
wvkimage_list_pop_front(
    _WVkImageList * self
);

// Also call the destory function of _WVkImage
qo_size_t
wvkimage_list_clear(
    _WVkImageList * self
);
