#include "graphics.h"
#include "platform.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct window_t {
  Display *display;
  Window window;
  Atom wm_delete_window;
  void (*event_callback)(int event, void *data);

  XImage *ximage;
  XShmSegmentInfo shminfo;
  bool use_shm;
  GC gc;
};

window_t *window_create(const char *title, int width, int height, void (*event_callback)(int event, void *data)) {
  window_t *w = calloc(1, sizeof(window_t));
  w->display = XOpenDisplay(NULL);
  if (!w->display) { free(w); return NULL; }

  int screen = DefaultScreen(w->display);
  w->window = XCreateSimpleWindow(w->display, RootWindow(w->display, screen), 10, 10, width, height, 1, BlackPixel(w->display, screen), WhitePixel(w->display, screen));
  XStoreName(w->display, w->window, title);

  long event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                    FocusChangeMask | StructureNotifyMask | EnterWindowMask |
                    LeaveWindowMask | PropertyChangeMask;
  XSelectInput(w->display, w->window, event_mask);
  w->wm_delete_window = XInternAtom(w->display, "WM_DELETE_WINDOW", False);
  
  XSetWMProtocols(w->display, w->window, &w->wm_delete_window, 1);
  XMapWindow(w->display, w->window);


  w->event_callback = event_callback;
  return w;
}

void window_create_image(window_t *w, int width, int height) {
    XWindowAttributes wa;
    XGetWindowAttributes(w->display, w->window, &wa);
    w->gc = XCreateGC(w->display, w->window, 0, NULL);

    w->use_shm = XShmQueryExtension(w->display);

    if (w->use_shm) {
        w->shminfo.shmid = shmget(IPC_PRIVATE, width*height*sizeof(u32), IPC_CREAT|0777);
        if (w->shminfo.shmid < 0) {
            w->use_shm = false;
        } else {
            w->shminfo.shmaddr = shmat(w->shminfo.shmid, 0, 0);
            w->shminfo.readOnly = False;
            w->ximage = XShmCreateImage(w->display, wa.visual, wa.depth,
                                        ZPixmap, (char*) w->shminfo.shmaddr,
                                        &w->shminfo, width, height);
            XShmAttach(w->display, &w->shminfo);
        }
    }

    if (!w->use_shm) {
        u32 *pixels = calloc(width*height, sizeof(u32));
        w->ximage = XCreateImage(w->display, wa.visual, wa.depth, ZPixmap, 0, (char*) pixels, width, height, 32, width*sizeof(u32));
    }
}

void window_draw_framebuffer(window_t *w, render_context *ctx) {
    u32 *src = ctx->framebuffer.color_buffer;
    int w_fb = ctx->framebuffer.width;
    int h_fb = ctx->framebuffer.height;

    char *dst = w->ximage->data;
    int bytes_per_line = w->ximage->bytes_per_line;

    for (int y = 0; y < h_fb; y++) {
        memcpy(dst + y * bytes_per_line, src + y * w_fb, w_fb * sizeof(u32));
    }

    if (w->use_shm) XShmPutImage(w->display, w->window, w->gc, w->ximage, 0, 0, 0, 0, w_fb, h_fb, True);
    else XPutImage(w->display, w->window, w->gc, w->ximage, 0, 0, 0, 0, w_fb, h_fb);

    XFlush(w->display);
}

static keycode_t translate_keysym(KeySym sym) {
    switch (sym) {
        case XK_Escape: return KEY_ESCAPE;
        case XK_Return: return KEY_ENTER;
        case XK_space:  return KEY_SPACE;
        case XK_Up:     return KEY_UP;
        case XK_Down:   return KEY_DOWN;
        case XK_Left:   return KEY_LEFT;
        case XK_Right:  return KEY_RIGHT;
        //LSHIFT RSHIFT LCTRL RCTRL TAB
        case XK_a: case XK_A: return KEY_A;
        case XK_b: case XK_B: return KEY_B;
        case XK_c: case XK_C: return KEY_C;
        case XK_d: case XK_D: return KEY_D;
        case XK_e: case XK_E: return KEY_E;
        case XK_f: case XK_F: return KEY_F;
        case XK_g: case XK_G: return KEY_G;
        case XK_h: case XK_H: return KEY_H;
        case XK_i: case XK_I: return KEY_I;
        case XK_j: case XK_J: return KEY_J;
        case XK_k: case XK_K: return KEY_K;
        case XK_l: case XK_L: return KEY_L;
        case XK_m: case XK_M: return KEY_M;
        case XK_o: case XK_O: return KEY_O;
        case XK_p: case XK_P: return KEY_P;
        case XK_q: case XK_Q: return KEY_Q;
        case XK_r: case XK_R: return KEY_R;
        case XK_s: case XK_S: return KEY_S;
        case XK_t: case XK_T: return KEY_T;
        case XK_u: case XK_U: return KEY_U;
        case XK_v: case XK_V: return KEY_V;
        case XK_w: case XK_W: return KEY_W;
        case XK_x: case XK_X: return KEY_X;
        case XK_y: case XK_Y: return KEY_Y;
        case XK_z: case XK_Z: return KEY_Z;
        // NUMBERS
        default: return KEY_UNKNOWN;
    }
}

void window_poll_events(window_t *w) {
    while (XPending(w->display)) {
        XEvent ev;
        XNextEvent(w->display, &ev);

        switch (ev.type) {
            case ClientMessage:
                if ((Atom)ev.xclient.data.l[0] == w->wm_delete_window) {
                    w->event_callback(EVENT_WINDOW_CLOSE, NULL);
                }
                break;

            case Expose:
                w->event_callback(EVENT_WINDOW_EXPOSE, NULL);
                break;

            case FocusIn:
                w->event_callback(EVENT_WINDOW_FOCUS_GAINED, NULL);
                break;

            case FocusOut:
                w->event_callback(EVENT_WINDOW_FOCUS_LOST, NULL);
                break;

            case EnterNotify:
                w->event_callback(EVENT_MOUSE_ENTER, NULL);
                break;

            case LeaveNotify:
                w->event_callback(EVENT_MOUSE_LEAVE, NULL);
                break;

            case ConfigureNotify: {
                event_window_resize_t e = { ev.xconfigure.width, ev.xconfigure.height };
                w->event_callback(EVENT_WINDOW_RESIZE, &e);
            } break;

            case KeyPress: {
                KeySym sym = XLookupKeysym(&ev.xkey, 0);
                keycode_t code = translate_keysym(sym); // helper function
                event_key_t key = { code };
                w->event_callback(EVENT_KEY_DOWN, &key);
            } break;

            case KeyRelease: {
                KeySym sym = XLookupKeysym(&ev.xkey, 0);
                keycode_t code = translate_keysym(sym);
                event_key_t key = { code };
                w->event_callback(EVENT_KEY_UP, &key);
            } break;

            case ButtonPress: {
                if (ev.xbutton.button == Button4 || ev.xbutton.button == Button5) {
                    event_mouse_wheel_t wheel = { ev.xbutton.x, ev.xbutton.y, 0, 0 };
                    wheel.dy = (ev.xbutton.button == Button4) ? 1 : -1;
                    w->event_callback(EVENT_MOUSE_WHEEL, &wheel);
                } else {
                    event_mouse_button_t mb = { ev.xbutton.x, ev.xbutton.y, ev.xbutton.button };
                    w->event_callback(EVENT_MOUSE_BUTTON_DOWN, &mb);
                }
            } break;

            case ButtonRelease: {
                event_mouse_button_t mb = { ev.xbutton.x, ev.xbutton.y, ev.xbutton.button };
                w->event_callback(EVENT_MOUSE_BUTTON_UP, &mb);
            } break;

            case MotionNotify: {
                event_mouse_move_t mm = { ev.xmotion.x, ev.xmotion.y };
                w->event_callback(EVENT_MOUSE_MOVE, &mm);
            } break;
        }
    }
}

void window_destroy(window_t *w) {
  if (!w) return;
  XDestroyWindow(w->display, w->window);
  XCloseDisplay(w->display);
}
