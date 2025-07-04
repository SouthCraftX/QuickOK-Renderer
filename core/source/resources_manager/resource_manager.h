#pragma once
#define __QOR_RESOURCE_MANAGER_SRC__

#include "../visual_element.h"
#include "../wrapped_vulkan_objects/everything.h"
#include "../container/id_object_map.h"
#include "sampler_cache.h"

struct __ResourceManager
{
    _VkDeviceContext * device_context;
    _IDObjectMap       mesh_map;
    _IDObjectMap       wvkimage_map;
    _IDObjectMap       wvktexture_map;
    _IDObjectMap       wvkshader_map;
    _SamplerCache      sampler_cache;
    // TODO: Map of MaterialTemplate
};
typedef struct __ResourceManager _ResourceManager;

#define DECL_ID_QUERY_FN(x) \
        QO_NODISCARD QO_NONNULL(1) \
        object_ptr_t \
        query_ ## x( \
    _ResourceManager const * self , object_id_t id \
        )

DECL_ID_QUERY_FN(texture);
DECL_ID_QUERY_FN(mesh);
DECL_ID_QUERY_FN(shader);
DECL_ID_QUERY_FN(image);