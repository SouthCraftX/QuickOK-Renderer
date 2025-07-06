#pragma once
#define __QOR_STRING_HASH_WVKTEXTURE_MAP_SRC__

#include "funnel_hash_table.h"
#include "../wrapped_vulkan_objects/wvktexture.h"

struct __StringHashWVkTextureMap
{
    _FunnelHashTable  table;
};
typedef struct  __StringHashWVkTextureMap _StringHashWVkTextureMap;
QO_NODISCARD
qo_stat_t
string_hash_wvktexture_map_init(
    _StringHashWVkTextureMap * self
);

void
string_hash_wvktexture_map_destroy(
    _StringHashWVkTextureMap * self
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_wvktexture_map_search(
    _StringHashWVkTextureMap * self ,
    string_hash_t              string_hash ,
    _WVkTexture **             p_texture
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_wvktexture_map_insert(
    _StringHashWVkTextureMap * self ,
    string_hash_t              string_hash ,
    _WVkTexture *              texture
);

QO_NODISCARD  QO_NONNULL(1 , 3)
qo_bool_t
string_hash_wvktexture_map_set(
    _StringHashWVkTextureMap * self ,
    string_hash_t              string_hash ,
    _WVkTexture *              texture
);
