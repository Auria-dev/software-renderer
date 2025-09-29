#include "platform.h"

struct window_t {
    SDL_Window*   sdl_window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    SDL_Rect      rect;
    viewport_t*   viewport;
};

window_t* g_window = NULL;

bool window_init(char* title, int window_width, int window_height, viewport_t* vp) {
    g_window = malloc(sizeof(window_t));
    if (!g_window) {
        fprintf(stderr, "failed to allocate g_window\n");
        return false;
    }

    g_window->viewport = vp;

    g_window->rect.x = 0;
    g_window->rect.y = 0;
    g_window->rect.w = window_width;
    g_window->rect.h = window_height;
       
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    SDL_DisplayMode SDLDM;
    SDL_GetCurrentDisplayMode(0, &SDLDM);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_WARP_MOTION, "1");

    g_window->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (!g_window->sdl_window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    g_window->renderer = SDL_CreateRenderer(g_window->sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_window->renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    size_t color_size = g_window->viewport->width * g_window->viewport->height  * sizeof(uint32_t);
    size_t z_size     = g_window->viewport->width * g_window->viewport->height * sizeof(float);
    posix_memalign((void**)&g_window->viewport->color_buffer, 32, color_size);
    posix_memalign((void**)&g_window->viewport->depth_buffer, 32, z_size);    
    if (!g_window->viewport->color_buffer || !g_window->viewport->depth_buffer) {
        fprintf(stderr, "Error allocating memory for buffers.\n");
        return false;
    }

    g_window->texture = SDL_CreateTexture(g_window->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_window->viewport->width, g_window->viewport->height);
    if (!g_window->texture) {
        fprintf(stderr, "Error creating SDL texture.\n");
        return false;
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    window_resize();
    return true;

}

void window_resize(void) {
    int window_w, window_h;
    SDL_GetWindowSize(g_window->sdl_window, &window_w, &window_h);

    static int orig_w = 0, orig_h = 0;
    if (!orig_w) {
        orig_w = g_window->viewport->width;
        orig_h = g_window->viewport->height;
    }

    int scale_w = window_w / orig_w;
    int scale_h = window_h / orig_h;
    int scale   = (scale_w < scale_h ? scale_w : scale_h);
    if (scale < 1) scale = 1;

    g_window->viewport->width  = window_w / scale;
    g_window->viewport->height = window_h / scale;
    g_window->viewport->half_width  = g_window->viewport->width  * 0.5f;
    g_window->viewport->half_height = g_window->viewport->height * 0.5f;

    g_window->rect.x = 0;
    g_window->rect.y = 0;
    g_window->rect.w = window_w;
    g_window->rect.h = window_h;

    if (g_window->texture) SDL_DestroyTexture(g_window->texture);
    g_window->texture = SDL_CreateTexture(
        g_window->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        g_window->viewport->width, g_window->viewport->height
    );

    size_t color_size = g_window->viewport->width * g_window->viewport->height  * sizeof(uint32_t);
    size_t z_size     = g_window->viewport->width * g_window->viewport->height * sizeof(float);
    posix_memalign((void**)&g_window->viewport->color_buffer, 32, color_size);
    posix_memalign((void**)&g_window->viewport->depth_buffer, 32, z_size);    
}

void window_frame_start() {    
    memset(g_window->viewport->color_buffer, 0xff222222, g_window->viewport->width*g_window->viewport->height*sizeof(u32));
    memset(g_window->viewport->depth_buffer, 0, g_window->viewport->width*g_window->viewport->height*sizeof(float));
}

void window_frame_render(void) {
    SDL_UpdateTexture(g_window->texture, NULL, g_window->viewport->color_buffer, g_window->viewport->width * sizeof(u32));
    SDL_SetRenderDrawColor(g_window->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_window->renderer);
    SDL_RenderSetViewport(g_window->renderer, &g_window->rect);
    SDL_RenderCopy(g_window->renderer, g_window->texture, NULL, NULL);
    SDL_RenderPresent(g_window->renderer);
}

void window_destroy(void) {
    free(g_window->viewport->color_buffer);
    free(g_window->viewport->depth_buffer);
    SDL_DestroyTexture(g_window->texture);
    SDL_DestroyRenderer(g_window->renderer);
    SDL_DestroyWindow(g_window->sdl_window);
    IMG_Quit();
    SDL_Quit();
}


