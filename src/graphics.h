#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "c3m.h"
#include "vertex.h"
#include "materials.h"
#include "clipping.h"

enum {
  GBUFFER_INDEX,
  GBUFFER_VERTEX
};

typedef struct {
  void*  data;
  size_t size;
  u32    type;
} buffer_t;

enum {
    FRUSTUM_LEFT,
    FRUSTUM_RIGHT,
    FRUSTUM_TOP,
    FRUSTUM_BOTTOM,
    FRUSTUM_NEAR,
    FRUSTUM_FAR
};

typedef struct {
    clipping_plane_t planes[6];
} frustum_t;

typedef struct {
  u32* color_buffer;
  float* depth_buffer;
  int width, height;
} framebuffer_t;

typedef enum {
    SHADER_SGT, // scalar gouraud textured
    SHADER_SGC, // scalar gouraud colored
    SHADER_SFC, // scalar flat colored
    SHADER_SFT, // scalar flat textured
    // TODO: simd
} shader_type_t;

typedef struct render_context {
  mat4 projection_matrix;
  mat4 world_matrix;
  mat4 view_matrix;

  frustum_t frustum;

  framebuffer_t framebuffer;

  buffer_t vertex_buffer;
  buffer_t index_buffer;
  int material_id;
  
  material_manager_t material_manager;

  float clip_near;
  float clip_far;

  bool depth_test;
  bool blend_test;
  bool cull_face;
} render_context;

void draw_pixel(render_context *ctx, int x, int y, u32 c);

framebuffer_t framebuffer_init(int width, int height);
frustum_t frustum_init(float fov, float aspect_ratio, float clipping_near, float clipping_far);

render_context render_context_init(
    int width, int height,
    float fov, float aspect_ratio, float near, float far,
    vec3 cam_pos, vec3 cam_target, vec3 cam_up,
    bool enable_depth_test, bool enable_blend_test, bool enable_cull_face);

void g_update_projection_matrix(render_context *ctx, float fov, float ar);
void g_update_view_matrix(render_context *ctx, mat4 view);
void g_update_world_matrix(render_context *ctx, vec3 position, vec3 rotation, vec3 scale);

void g_bind_material(render_context *ctx, int material_id);
void g_bind_buffer(render_context *ctx, u32 type, void* data, size_t size);

void g_draw_elements(render_context *ctx, u32 count, u32 *indices);

void draw_triangle(
        render_context* ctx,
        shader_type_t shader_type,
        float x0, float y0, float w0, float u0, float v0, u32 c0,
        float x1, float y1, float w1, float u1, float v1, u32 c1,
        float x2, float y2, float w2, float u2, float v2, u32 c2);

void draw_triangle_scalar(
        render_context *ctx,
        float x0, float y0, float w0, float u0, float v0, u32 c0,
        float x1, float y1, float w1, float u1, float v1, u32 c1,
        float x2, float y2, float w2, float u2, float v2, u32 c2);

#endif // GRAPHICS_H