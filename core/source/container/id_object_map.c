#include "id_object_map.h"
#include "funnel_hash_table.h"
#include <xxh3.h>

qo_bool_t
id_equals(
    fht_key_t  x ,
    fht_key_t  y
) {
    return x == y;
}

XXH64_hash_t
id_hash(
    fht_key_t    key ,
    qo_uint32_t  salt
) {
    return key ^ salt;
}

qo_stat_t
idobject_map_init(
    _IDObjectMap * self
) {
    return fht_init(
        &self->hash_table , 0 , .1 , &(_FunnelHashTableAuxiliary) {
        .key_destroy_func = NULL ,
        .equals_func = id_equals ,
        .hash_func = id_hash
    }
    );
}

void
idobject_map_destroy(
    _IDObjectMap * self
) {
    fht_destroy(&self->hash_table);
}

qo_bool_t
idobject_map_insert(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t   object
) {
    return fht_insert(&self->hash_table , id , (fht_value_t) object);
}

qo_bool_t
idobject_map_search(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t * p_object_ptr
) {
    return fht_search(&self->hash_table , id , (fht_value_t *) p_object_ptr);
}

qo_bool_t
idobject_map_set(
    _IDObjectMap * self ,
    object_id_t    id ,
    object_ptr_t   object
) {
    return fht_set(&self->hash_table , id , (fht_value_t) object);
}
