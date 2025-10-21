#include "clipping.h"

static float signed_distance(const vertex_t* vertex, const clipping_plane_t* plane) {
    return vec3_dot(plane->normal, vertex->position) - vec3_dot(plane->normal, plane->point);
}

static u32 color_lerp(u32 color1, u32 color2, float t) {
    u32 r1 = (color1 >> 24) & 0xFF;
    u32 g1 = (color1 >> 16) & 0xFF;
    u32 b1 = (color1 >> 8) & 0xFF;
    u32 a1 = color1 & 0xFF;

    u32 r2 = (color2 >> 24) & 0xFF;
    u32 g2 = (color2 >> 16) & 0xFF;
    u32 b2 = (color2 >> 8) & 0xFF;
    u32 a2 = color2 & 0xFF;

    u32 r = (u32)lerpf((float)r1, (float)r2, t);
    u32 g = (u32)lerpf((float)g1, (float)g2, t);
    u32 b = (u32)lerpf((float)b1, (float)b2, t);
    u32 a = (u32)lerpf((float)a1, (float)a2, t);

    return (r << 24) | (g << 16) | (b << 8) | a;
}

static vertex_t intersect_plane(const vertex_t* v1, const vertex_t* v2, const clipping_plane_t* plane) {
    float d1 = signed_distance(v1, plane);
    float d2 = signed_distance(v2, plane);
    float t = d1 / (d1 - d2);

    vertex_t out;
    out.position = vec3_lerp(v1->position, v2->position, t);
    out.normal = vec3_lerp(v1->normal, v2->normal, t);
    out.texcoord = vec2_lerp(v1->texcoord, v2->texcoord, t);   
    out.color = color_lerp(v1->color, v2->color, t);
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