#include "clipping.h"

static float signed_distance(const vertex_t* vertex, const clipping_plane_t* plane) {
    return vec3_dot(plane->normal, vertex->position) - vec3_dot(plane->normal, plane->point);
}

static vertex_t intersect_plane(const vertex_t* v1, const vertex_t* v2, const clipping_plane_t* plane) {
    float d1 = signed_distance(v1, plane);
    float d2 = signed_distance(v2, plane);
    float t = d1 / (d1 - d2);

    vertex_t out;
    out.position = vec3_lerp(v1->position, v2->position, t);
    out.normal = vec3_lerp(v1->normal, v2->normal, t);
    out.texcoord = vec2_lerp(v1->texcoord, v2->texcoord, t);
    return out;
}

int clip_polygon_against_plane(vertex_t* polygon_vertices, int num_vertices, vertex_t* clipped_polygon_vertices, const clipping_plane_t* plane) {
    int num_clipped_vertices = 0;

    for (int i = 0; i < num_vertices; i++) {
        vertex_t* current_vertex = &polygon_vertices[i];
        vertex_t* prev_vertex = &polygon_vertices[(i + num_vertices - 1) % num_vertices];

        float current_dot = signed_distance(current_vertex, plane);
        float prev_dot = signed_distance(prev_vertex, plane);

        bool is_current_inside = current_dot >= 0;
        bool is_prev_inside = prev_dot >= 0;

        if (is_current_inside) {
            if (!is_prev_inside) {
                if (num_clipped_vertices < MAX_POLYGON_VERTICES) {
                    clipped_polygon_vertices[num_clipped_vertices++] = intersect_plane(prev_vertex, current_vertex, plane);
                }
            }
            if (num_clipped_vertices < MAX_POLYGON_VERTICES) {
                clipped_polygon_vertices[num_clipped_vertices++] = *current_vertex;
            }
        } else if (is_prev_inside) {
            if (num_clipped_vertices < MAX_POLYGON_VERTICES) {
                clipped_polygon_vertices[num_clipped_vertices++] = intersect_plane(prev_vertex, current_vertex, plane);
            }
        }
    }

    return num_clipped_vertices;
}
