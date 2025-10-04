#include "graphics.h"

render_context render_context_init() {
  render_context ctx = {0};
  ctx.projection_matrix = mat4_identity();
  ctx.world_matrix = mat4_identity();
  ctx.model_matrix = mat4_identity();
  ctx.framebuffer = framebuffer_init(640, 480);
  ctx.vertex_buffer.data = NULL;
  ctx.vertex_buffer.size = 0;
  ctx.vertex_buffer.type = GBUFFER_VERTEX;
  ctx.index_buffer.data = NULL;
  ctx.index_buffer.size = 0;
  ctx.index_buffer.type = GBUFFER_INDEX;
  ctx.texture_manager = tm_init();
  ctx.depth_test = true;
  ctx.blend_test = false;
  ctx.cull_face = false;
  return ctx;
}

framebuffer_t framebuffer_init(int width, int height) {
  framebuffer_t fb;
  fb.width = width;
  fb.height = height;
  fb.color_buffer = calloc(fb.width * fb.height, sizeof(u32));
  fb.depth_buffer = calloc(fb.width * fb.height, sizeof(float));
  return fb;
}

void draw_pixel(render_context *ctx, int x, int y, u32 c) {
  if (x < 0 || x >= ctx->framebuffer.width || y < 0 || y >= ctx->framebuffer.height) return;
  ctx->framebuffer.color_buffer[y * ctx->framebuffer.width + x] = c;
}
