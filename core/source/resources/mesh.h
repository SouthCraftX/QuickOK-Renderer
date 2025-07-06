#pragma once
#define __QOR_MESH_SRC__

#include "../wrapped_vulkan_objects/wvkbuffer.h"

struct __Mesh
{
    object_id_t        id; // Equal to Renderable's mesh_id
    qo_uint32_t        index_count;
    qo_uint32_t        vertex_count;
    _VkDeviceContext * device_context;
    qo_cstring_t       path; // its lifetime is aligned with _Mesh's lifetime
    state_id_t         vertex_input_state_id;
};
typedef struct __Mesh _Mesh;

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)

qo_uint32_t
mesh_get_vertex_count(
    _Mesh * self
) {
    return self->vertex_count;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)

qo_uint32_t
mesh_get_index_count(
    _Mesh * self
) {
    return self->index_count;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)

state_id_t
mesh_get_vertex_input_state_id(
    _Mesh * self
) {
    return self->vertex_input_state_id;
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE QO_NODISCARD QO_NONNULL(1)

object_id_t
mesh_get_id(
    _Mesh * self
) {
    return self->id;
}

QO_NONNULL(1)
_WVkBuffer *
mesh_get_vertex_buffer(
    _Mesh * self
);

QO_NONNULL(1)
_WVkBuffer *
mesh_get_index_buffer(
    _Mesh * self
);
