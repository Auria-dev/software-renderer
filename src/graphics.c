#include "graphics.h"

extern viewport_t g_viewport;

void draw_pixel(int x, int y, u32 c) {
  if (x<0 || y<0 || x>g_viewport.width || y>g_viewport.height) return;
  g_viewport.color_buffer[y * g_viewport.width + x] = c;  
}
