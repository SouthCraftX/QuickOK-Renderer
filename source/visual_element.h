#pragma once
#define __QOR_VISUAL_ELEMENT_SRC__

#include "rendering_env.h"

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

struct __Mesh
{
    mesh_id_t id; // Equal to Renderable's mesh_id
    qo_uint32_t index_count;
    qo_uint32_t vertex_count;
    qo_cstring_t path; // its lifetime is aligned with _Mesh's lifetime
};
typedef struct __Mesh _Mesh;

// The minimal renderable object
struct __IRenderable;
typedef struct __IRenderable _IRenderable;
struct __IRenderable
{
    material_id_t 
    (* get_material_id)(
        _IRenderable* renderable
    );

    mesh_id_t
    (* get_mesh_id)(
        _IRenderable* renderable
    );
};

struct __IVisualElement
{
};
typedef struct __IVisualElement _IVisualElement;
