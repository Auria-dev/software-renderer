#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "platform.h"
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
  mat4 model_matrix;

  int width, height;

  framebuffer_t framebuffer;

  buffer_t vertex_buffer;
  buffer_t index_buffer;
  
  texture_manager_t texture_manager;

  bool depth_test;
  bool blend_test;
  bool cull_face;

} render_context;

void draw_pixel(int x, int y, u32 c);

#endif
