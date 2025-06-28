#include "rendering_env.h"
#include <cglm/cglm.h>
#include <cglm/types.h>

struct __ClassicVertex
{
    vec3    position;
    vec3    color;
    vec2    texcoord;
    vec3    tangent;
};
typedef struct __ClassicVertex _ClassicVertex;

void
clasic_vertex_make_layout(
    _ClassicVertex *    self 
    // TODO:
);