#include "main.h"
#include "graphics.h"

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480

static bool running = false;
static vec2 mousepos;

void handle_event(int event, void *data) {
  switch (event) {
    case EVENT_WINDOW_CLOSE: {
      running = false;
    } break;
    case EVENT_KEY_DOWN: {
      event_key_t *e = data;
      printf("key down: %d\n", e->code);
      if (e->code == KEY_ESCAPE) {
        running = false;
      }
    } break;
    case EVENT_KEY_UP: {
      event_key_t *e = data;
      printf("key up: %d\n", e->code);
    } break;
    case EVENT_MOUSE_MOVE: {
      event_mouse_move_t*e = data;
      mousepos.x = e->x;
      mousepos.y = e->y;
    } break;
  }
}

int main(int argc, char *argv[]) {
  window_t *win = window_create("Hello :D", 800, 600, handle_event);
  if (!win) { fprintf(stderr, "Failed to create window\n"); return 1; }

  render_context ctx = {0};
  ctx.framebuffer = framebuffer_init(640, 480);
  window_create_image(win, ctx.framebuffer.width, ctx.framebuffer.height);
  
  running = true;
  struct timespec last_time;
  clock_gettime(CLOCK_MONOTONIC, &last_time);
  int frames = 0;

  while (running) {
    window_poll_events(win);

    static int frame = 0;
    memset(ctx.framebuffer.color_buffer,(int)0xff222222, ctx.framebuffer.width*ctx.framebuffer.height*sizeof(u32));

    for (int x=0;x<640;x++) {
      for (int y=0;y<480;y++) {
        u32 c = 0xff << 24 | x%255<<16 | y%255<<8 | 0x00;
        draw_pixel(&ctx, x,y, c);
      }
    }
    draw_pixel(&ctx, mousepos.x-5,mousepos.y-5, 0xff00ffff);

    frame++;
    window_draw_framebuffer(win, &ctx);
    frames++;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) * 1e-9;
    if (elapsed >= 1.0) {
      printf("FPS: %d\n", frames);
      frames = 0;
      last_time = now;
    }
  }

  window_destroy(win);
  return 0;
}
