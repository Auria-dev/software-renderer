#ifndef MESH_H
#define MESH_H

#include "c3m.h"

typedef struct {
    u32* indices;
    int index_count;
    int material_id;
} submesh_t;

typedef struct {
    vec3* vertices;
    vec3* normals;
    vec2* uvs;
    int vertex_count;

    submesh_t* submeshes;
    int submesh_count;

    vec3 position;
    vec3 rotation;
    vec3 scale;

    // TODO: bounding box for frustum culling
    // TODO: skeletal animation data
    // TODO: collision data for eventual physics engine
} mesh_t;

#endif
