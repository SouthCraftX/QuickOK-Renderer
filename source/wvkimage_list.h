#include "wrapped_vulkan_image.h"

struct __WVkImageListNode;
typedef struct __WVkImageListNode _WVkImageListNode;
struct __WVkImageListNode
{
    _WVkImage *                 wvkimage;
    _WVkImageListNode *         next;
    _WVkImageListNode *         prev;
};

struct __WVkImageList
{
    _WVkImageListNode *         head;
    _WVkImageListNode *         tail;
    _WVkImageListNode *         current;
    qo_uint32_t                 count;
};
typedef struct __WVkImageList _WVkImageList;

_WVkImageListNode *
wvkimage_list_node_create(
    _WVkImage * wvkimage
) {
    _WVkImageListNode * node = (_WVkImageListNode *)mi_malloc(
        sizeof(_WVkImageListNode)
    );
    node->wvkimage = wvkimage;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

void
wvkimage_list_node_destroy(
    _WVkImageListNode * node
) {
    mi_free(node);
}

void
wvkimage_list_init(
    _WVkImageList * list
) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void
wvkimage_list_destroy(
    _WVkImageList * list
) {
    _WVkImageListNode * node = list->head;
    while (node != NULL)
    {
        _WVkImageListNode * next = node->next;
        wvkimage_list_node_destroy(node);
        node = next;
    }
}

void
wvkimage_list_insert_node_ahead(
    _WVkImageList * list ,
    _WVkImageListNode * node
) {
    if (list->count == 0)
    {
        list->head = node;
        list->tail = node;
        list->count = 1;
        return;
    }
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
    list->count++;
}

void
wvkimage_list_insert_node_behind(
    _WVkImageList * list ,
    _WVkImageListNode * node
) {
    if (list->count == 0)
    {
        list->head = node;
        list->tail = node;
        list->count = 1;
        return;
    }
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
    list->count++;
}

qo_bool_t
wvkimage_list_remove_node_ahead(
    _WVkImageList * list
) {
    if (list->current->next)
    {
        _WVkImageListNode * next = list->current->next;
        list->current->next = next->next;
        wvkimage_list_node_destroy(next);
        list->count--;
        return qo_true;
    }
    return qo_false;
}

qo_bool_t
wvkimage_list_insert_image_ahead(
    _WVkImageList * list ,
    _WVkImage *     image
) {
    _WVkImageListNode * node = wvkimage_list_node_create(image);
    if (node)
    {
        wvkimage_list_insert_node_ahead(list, node);
        return qo_true;
    }
    return qo_false;
}
