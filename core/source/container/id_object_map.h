#pragma once
#define __QOR_ID_OBJECT_MAP_SRC__

#include "funnel_hash_table.h"

typedef qo_flag64_t          object_id_t;
typedef struct _QOR_Object * object_ptr_t;

struct __IDObjectMap
{
    _FunnelHashTable  hash_table;
};
typedef struct __IDObjectMap _IDObjectMap;

QO_NODISCARD  QO_NONNULL(1)
qo_stat_t
idobject_map_init(
    _IDObjectMap * self
);

QO_NODISCARD  QO_NONNULL(1)
qo_bool_t
idobject_map_search(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t * p_object_ptr
);

QO_NODISCARD  QO_NONNULL(1)
qo_bool_t
idobject_map_insert(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t   object
);

QO_NODISCARD  QO_NONNULL(1)
qo_bool_t
idobject_map_set(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t   object
);

void
idobject_map_destroy(
    _IDObjectMap * self
);
