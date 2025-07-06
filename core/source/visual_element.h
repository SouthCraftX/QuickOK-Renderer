#pragma once
#define __QOR_VISUAL_ELEMENT_SRC__

#include "rendering_env.h"
#include "classic_vertex.h"

typedef qo_uint64_t  sort_key_t;
typedef qo_uint64_t  material_id_t;
typedef qo_uint32_t  mesh_id_t;

// Temporary, stateless and transient
struct __Renderable
{
    sort_key_t     sort_key;
    material_id_t  material_id;
    mesh_id_t      mesh_id;
    qo_uint32_t     first_index;
    qo_int32_t      vertex_offset;
    qo_uint32_t     instance_count;
    qo_uint32_t     instance_data_offset;
    qo_uint32_t     instance_data_size;
};
typedef struct __Renderable _Renderable;


// The minimal renderable object
struct __IRenderable;
typedef struct __IRenderable _IRenderable;
struct __IRenderable
{
    material_id_t 
    (* get_material_id)(
        _IRenderable *  self
    );

    mesh_id_t
    (* get_mesh_id)(
        _IRenderable *  self
    );

    _Mesh const *
    (* get_mesh)(
        _IRenderable *  self
    );

    qo_size_t
    (* get_tags)(
        _IRenderable *  self ,
        qo_cstring_t *  tags
    );
};

struct __IVisualElement
{
};
typedef struct __IVisualElement _IVisualElement;

_Mesh *
mesh_new_from_path(
    qo_ccstring_t path
);

_Mesh *
mesh_new_from_data(
    _ClassicVertex *    vertices ,
    qo_uint32_t *       indices ,
    qo_uint32_t         vertices_count ,
    qo_uint32_t         indices_count
);
