#include "main.h"
#include "graphics.h"
#include <SDL2/SDL_keycode.h>

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480

viewport_t g_viewport;

int main(int argc, char *argv[]) {
    g_viewport = (viewport_t){
        .color_buffer = NULL,
        .depth_buffer = NULL,
        .width = RENDER_WIDTH,
        .height = RENDER_HEIGHT,
        .half_width = RENDER_WIDTH * 0.5f,
        .half_height = RENDER_HEIGHT * 0.5f
    };
    
    window_init("Titl 2e", 640, 480, &g_viewport);
    
    int running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    running = false;      
                } break;
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) window_resize();
                } break;
                case SDL_KEYUP: {
                    if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                } break;
            }
        }

        window_frame_start();

        draw_pixel(60, 60, 0xffff0000);
        
        window_frame_render();
    }

    window_destroy();
    return 0;
}
