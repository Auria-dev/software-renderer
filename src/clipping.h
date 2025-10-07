#ifndef CLIPPING_H
#define CLIPPING_H

#include "graphics.h"

#define MAX_POLYGON_VERTICES 10

// Clips a polygon against a single plane.
// `num_vertices` is the number of vertices in the input polygon.
// `polygon_vertices` is the array of vertices for the input polygon.
// `clipped_polygon_vertices` is an output array to store the vertices of the clipped polygon.
// Returns the number of vertices in the clipped polygon.
int clip_polygon_against_plane(vertex_t* polygon_vertices, int num_vertices, vertex_t* clipped_polygon_vertices, const clipping_plane_t* plane);

#endif // CLIPPING_H
