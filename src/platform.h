#ifndef PLATFORM_H
#define PLATFORM_H

#include "c3m.h"
#include "graphics.h"
enum {
    EVENT_WINDOW_CLOSE,         // user clicked close / requested quit
    EVENT_WINDOW_RESIZE,        // window resized
    EVENT_WINDOW_MOVE,          // window moved
    EVENT_WINDOW_FOCUS_GAINED,  // window focused
    EVENT_WINDOW_FOCUS_LOST,    // window unfocused
    EVENT_WINDOW_MINIMIZE,      // window minimized
    EVENT_WINDOW_RESTORE,       // window restored from minimized/maximized
    EVENT_WINDOW_MAXIMIZE,      // window maximized
    EVENT_WINDOW_EXPOSE,        // needs redraw (e.g. uncovered or after minimize)
    
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_KEY_CHAR,             // text input (unicode character, not just keycode)

    EVENT_MOUSE_BUTTON_DOWN,
    EVENT_MOUSE_BUTTON_UP,
    EVENT_MOUSE_WHEEL,          // scroll wheel
    EVENT_MOUSE_MOVE,           // mouse moved
    EVENT_MOUSE_ENTER,          // cursor entered window
    EVENT_MOUSE_LEAVE,          // cursor left window
};

typedef enum {
    KEY_UNKNOWN = 0,
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_SPACE,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_TAB,
    KEY_A, KEY_B, KEY_C,
    KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L,
    KEY_M, KEY_N, KEY_O,
    KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X,
    KEY_Y, KEY_Z, KEY_1,
    KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7,
    KEY_8, KEY_9, KEY_0
    // TODO: more
} keycode_t;

typedef struct {
    int width, height;
} event_window_resize_t;

typedef struct {
    int x, y;
} event_window_move_t;

typedef struct {
    keycode_t code;
} event_key_t;

typedef struct {
    int x, y;
    int button;
} event_mouse_button_t;

typedef struct {
    int x, y;
    int dx, dy;
} event_mouse_wheel_t;

typedef struct {
    int x, y;
} event_mouse_move_t;

typedef struct window_t window_t;
window_t* window_create(const char* title, int width, int height, void (*event_callback)(int event, void* data));
void window_poll_events(window_t* window);
void window_destroy(window_t* window);

void window_create_image(window_t *w, int width, int height);
void window_draw_framebuffer(window_t *w, render_context *ctx);

// TODO: window_render window_resize window_rename? and mouse capturing with events?

#endif
