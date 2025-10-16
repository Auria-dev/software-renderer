#ifndef CLIPPING_H
#define CLIPPING_H

#include "graphics.h"

#define MAX_POLYGON_VERTICES 10

typedef struct {
    vec3 point;
    vec3 normal;
} clipping_plane_t;

int clip_polygon_against_plane(vertex_t* polygon_vertices, int num_vertices, vertex_t* clipped_polygon_vertices, const clipping_plane_t* plane);

#endif // CLIPPING_H
