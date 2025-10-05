#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "texture.h"

enum {
  GBUFFER_INDEX,
  GBUFFER_VERTEX
};

typedef struct {
  void*  data;
  size_t size;
  u32    type;
} buffer_t;

typedef struct {
  u32* color_buffer;
  float* depth_buffer;
  int width, height;
} framebuffer_t;

typedef struct render_context {
  mat4 projection_matrix;
  mat4 world_matrix;
  mat4 view_matrix;

  framebuffer_t framebuffer;

  buffer_t vertex_buffer;
  buffer_t index_buffer;
  int material_id;
  
  texture_manager_t texture_manager;
  // TODO: shader program

  float clip_near;
  float clip_far;

  bool depth_test;
  bool blend_test;
  bool cull_face;

} render_context;

// drawing functions
void draw_pixel(render_context *ctx, int x, int y, u32 c);

render_context render_context_init(
    int width, int height,
    float fov, float aspect_ratio, float near, float far,
    vec3 cam_pos, vec3 cam_target, vec3 cam_up,
    bool enable_depth_test, bool enable_blend_test, bool enable_cull_face);

framebuffer_t framebuffer_init(int width, int height);

void g_bind_material(render_context *ctx, int material_id);
void g_bind_buffer(render_context *ctx, u32 type, void* data, size_t size);
void g_buffer_data(render_context *ctx, u32 type, void* data, size_t size);
void g_draw_elements(render_context *ctx, u32 count, u32* indices);

#endif
