#include "string_hash_index_map.h"
#include "funnel_hash_table.h"

XXH64_hash_t
string_hash(
    fht_key_t key , //< Already hash of string
    qo_uint32_t salt
) {
    return key ^ (qo_uint64_t) salt;
}

qo_bool_t
string_hash_compare(
    fht_key_t   key1 ,
    fht_key_t   key2
) {
    return key1 == key2;
}

qo_stat_t
string_hash_wvkimage_index_map_init(
    _StringHash2WVkImageIndexMap *  self ,
    qo_uint32_t                    capacity
) {
    qo_stat_t status = fht_init(
        & self->table ,
        capacity ,
        .1f,
        &(_FunnelHashTableAuxiliary){
            .hash_func = string_hash,
            .equals_func = string_hash_compare,
            .key_destroy_func = NULL
        }
    );
    return status;
}

qo_bool_t
string_hash_wvkimage_index_map_search(
    _StringHash2WVkImageIndexMap *  self ,
    string_hash_t                   key ,
    _ActiveImageIndexEntry *                   p_index_entry
) {
    return fht_search(
        & self->table ,
        key ,
        (fht_key_t *)p_index_entry
    );
}

void
string_hash_wvkimage_index_map_destroy(
    _StringHash2WVkImageIndexMap *  self 
) {
    fht_destroy(& self->table);
}

qo_bool_t
string_hash_wvkimage_index_map_set(
    _StringHash2WVkImageIndexMap *  self ,
    string_hash_t                   key ,
    _ActiveImageIndexEntry          index_entry
) {
    return fht_set(
        & self->table ,
        key ,
        (fht_key_t)&index_entry
    );
}


