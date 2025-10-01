#include "graphics.h"

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
