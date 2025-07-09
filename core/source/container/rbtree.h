#pragma once
#define __QOR_RBTREE_SRC__

#include "../rendering_env.h"

typedef qo_intmax_t  rbt_key_t;
typedef qo_intmax_t  rbt_value_t;

typedef
    qo_bool_t
    (* rbt_compare_f)(
        rbt_key_t key1,
        rbt_key_t key2
    );

typedef
    void
    (* rbt_destroy_key_f)(
        rbt_key_t key
    );

typedef
    void
    (* rbt_destroy_value_f)(
        rbt_value_t value
    );


typedef enum
{
    RBTREE_RED ,
    RBTREE_BLACK
} _RBTreeColor;

typedef struct __RBTreeNode _RBTreeNode;
struct __RBTreeNode
{
    _RBTreeColor  color;
    rbt_key_t     key;
    rbt_value_t   value;
    _RBTreeNode * parent;
    _RBTreeNode * left;
    _RBTreeNode * right;
};

struct __RBTreeAuxiliary
{
    rbt_compare_f      equals_func;
    rbt_destroy_key_f  key_destroy_func;
    rbt_destroy_value_f value_destroy_func;
} 
typedef struct __RBTreeAuxiliary _RBTreeAuxiliary;

struct __RBTree
{
    _RBTreeNode *   root;
    _RBTreeAuxiliary auxiliary;
};  
typedef struct __RBTree _RBTree;

QO_NONNULL(1 , 2)
void
rbt_left_rotate(
    _RBTree * self,
    _RBTreeNode * node
);

QO_NONNULL(1 , 2)
void
rbt_right_rotate(
    _RBTree * self,
    _RBTreeNode * node
);

QO_NODISCARD
_RBTreeNode *
rbt_node_new(
    rbt_key_t key,
    rbt_value_t value
);

QO_NODISCARD
qo_bool_t
rbt_insert(
    _RBTree * self ,
    rbt_key_t key ,
    rbt_value_t value
);

void
rbt_node_destroy(
    _RBTree * self
);
