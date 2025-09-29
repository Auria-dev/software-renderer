#ifndef PLATFORM_H
#define PLATFORM_H

#include "c3m.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>

typedef struct {
    u32*   color_buffer;
    float* depth_buffer;
    int    width;
    int    height;
    int    half_width;
    int    half_height;
} viewport_t;

typedef struct window_t window_t;
extern window_t* g_window;

bool window_init(char* title, int window_width, int window_height, viewport_t* vp);
void window_resize(void);
void window_frame_start(void);
void window_frame_render(void);
void window_destroy(void);

#endif
