#ifdef _WIN32

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>

#include "platform.h"

struct window_t {
    HWND hwnd;
    HINSTANCE hInstance;
    void (*event_callback)(int event, void *data);
    framebuffer_t *fb;
    HBITMAP hbmp;
    HDC hdcMem;
    uint32_t *dib_pixels;
    int dib_width;
    int dib_height;
    RECT destRect;
    bool mouse_inside;
};

static const char *g_wincname = "software_renderer_window_class";
static bool g_class_registered = false;
static LRESULT CALLBACK platform_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static keycode_t vk_to_keycode(WPARAM vk) {
    switch (vk) {
        case VK_ESCAPE: return KEY_ESCAPE;
        case VK_RETURN: return KEY_ENTER;
        case VK_SPACE: return KEY_SPACE;
        case VK_UP: return KEY_UP;
        case VK_DOWN: return KEY_DOWN;
        case VK_LEFT: return KEY_LEFT;
        case VK_RIGHT: return KEY_RIGHT;
        case VK_LSHIFT: return KEY_LSHIFT;
        case VK_RSHIFT: return KEY_RSHIFT;
        case VK_LCONTROL: return KEY_LCTRL;
        case VK_RCONTROL: return KEY_RCTRL;
        case VK_TAB: return KEY_TAB;
        case 'A': return KEY_A;
        case 'B': return KEY_B;
        case 'C': return KEY_C;
        case 'D': return KEY_D;
        case 'E': return KEY_E;
        case 'F': return KEY_F;
        case 'G': return KEY_G;
        case 'H': return KEY_H;
        case 'I': return KEY_I;
        case 'J': return KEY_J;
        case 'K': return KEY_K;
        case 'L': return KEY_L;
        case 'M': return KEY_M;
        case 'N': return KEY_N;
        case 'O': return KEY_O;
        case 'P': return KEY_P;
        case 'Q': return KEY_Q;
        case 'R': return KEY_R;
        case 'S': return KEY_S;
        case 'T': return KEY_T;
        case 'U': return KEY_U;
        case 'V': return KEY_V;
        case 'W': return KEY_W;
        case 'X': return KEY_X;
        case 'Y': return KEY_Y;
        case 'Z': return KEY_Z;
        case '0': return KEY_0;
        case '1': return KEY_1;
        case '2': return KEY_2;
        case '3': return KEY_3;
        case '4': return KEY_4;
        case '5': return KEY_5;
        case '6': return KEY_6;
        case '7': return KEY_7;
        case '8': return KEY_8;
        case '9': return KEY_9;
        default:
            return KEY_UNKNOWN;
    }
}

static void create_dib_for_window(window_t *w, int width, int height) {
    if (w->hbmp) {
        SelectObject(w->hdcMem, GetStockObject(NULL_BRUSH));
        DeleteObject(w->hbmp);
        w->hbmp = NULL;
        w->dib_pixels = NULL;
    }

    HDC hdc = GetDC(w->hwnd);
    if (!w->hdcMem) w->hdcMem = CreateCompatibleDC(hdc);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bits = NULL;
    w->hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!w->hbmp) {
        w->dib_pixels = NULL;
        ReleaseDC(w->hwnd, hdc);
        return;
    }

    w->dib_pixels = (uint32_t *)bits;
    w->dib_width = width;
    w->dib_height = height;

    SelectObject(w->hdcMem, w->hbmp);
    ReleaseDC(w->hwnd, hdc);
}

window_t *window_create(const char *title, int width, int height, void (*event_callback)(int event, void *data)) {
    if (!g_class_registered) {
        WNDCLASSEXA wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpfnWndProc = platform_wndproc;
        wc.lpszClassName = g_wincname;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (!RegisterClassExA(&wc)) {
            return NULL;
        }
        g_class_registered = true;
    }

    HWND hwnd = CreateWindowExA(0, g_wincname, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!hwnd) return NULL;

    window_t *w = (window_t *)malloc(sizeof(window_t));
    memset(w, 0, sizeof(window_t));
    w->hwnd = hwnd;
    w->hInstance = GetModuleHandle(NULL);
    w->event_callback = event_callback;
    w->fb = NULL;
    w->hbmp = NULL;
    w->hdcMem = NULL;
    w->mouse_inside = false;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)w);

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    w->destRect.left = 0;
    w->destRect.top = 0;
    w->destRect.right = rcClient.right - rcClient.left;
    w->destRect.bottom = rcClient.bottom - rcClient.top;
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)(NULL));
    
    create_dib_for_window(w, max(1, width), max(1, height));

    return w;
}

void window_bind_framebuffer(window_t *w, framebuffer_t *fb) {
    if (!w) return;
    w->fb = fb;

    if (fb && fb->width > 0 && fb->height > 0) {
        create_dib_for_window(w, fb->width, fb->height);
        fb->color_buffer = w->dib_pixels;
    }
}

void window_blit(window_t *w) {
    if (!w) return;

    HDC hdc = GetDC(w->hwnd);
    if (!hdc) return;

    RECT client;
    GetClientRect(w->hwnd, &client);
    int clientWidth = client.right - client.left;
    int clientHeight = client.bottom - client.top;

    int srcW = (w->dib_width > 0) ? w->dib_width : clientWidth;
    int srcH = (w->dib_height > 0) ? w->dib_height : clientHeight;

    float scaleX = (float)clientWidth / (float)srcW;
    float scaleY = (float)clientHeight / (float)srcH;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    int newW = (int)(srcW * scale);
    int newH = (int)(srcH * scale);
    w->destRect.left = (clientWidth - newW) / 2;
    w->destRect.top = (clientHeight - newH) / 2;
    w->destRect.right = w->destRect.left + newW;
    w->destRect.bottom = w->destRect.top + newH;

    HBRUSH black = (HBRUSH)GetStockObject(BLACK_BRUSH);
    SetBkColor(hdc, RGB(0, 0, 0));

    RECT top_bar = {client.left, client.top, client.right, w->destRect.top};
    RECT bottom_bar = {client.left, w->destRect.bottom, client.right, client.bottom};
    RECT left_bar = {client.left, w->destRect.top, w->destRect.left, w->destRect.bottom};
    RECT right_bar = {w->destRect.right, w->destRect.top, client.right, w->destRect.bottom};
    
    if (top_bar.bottom > top_bar.top) ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &top_bar, NULL, 0, NULL);
    if (bottom_bar.bottom > bottom_bar.top) ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &bottom_bar, NULL, 0, NULL);
    if (left_bar.right > left_bar.left) ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &left_bar, NULL, 0, NULL);
    if (right_bar.right > right_bar.left) ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &right_bar, NULL, 0, NULL);

    if (w->hdcMem && w->hbmp) {
        if (newW == w->dib_width && newH == w->dib_height) {
            BitBlt(hdc, w->destRect.left, w->destRect.top, newW, newH, w->hdcMem, 0, 0, SRCCOPY);
        } else {
            SetStretchBltMode(hdc, COLORONCOLOR);
            StretchBlt(hdc, w->destRect.left, w->destRect.top, newW, newH, w->hdcMem, 0, 0, w->dib_width, w->dib_height, SRCCOPY);
        }
    }

    ReleaseDC(w->hwnd, hdc);
}

void window_destroy(window_t *window) {
    if (!window) return;
    if (window->hbmp) {
        SelectObject(window->hdcMem, GetStockObject(NULL_BRUSH));
        DeleteObject(window->hbmp);
        window->hbmp = NULL;
    }
    if (window->hdcMem) {
        DeleteDC(window->hdcMem);
        window->hdcMem = NULL;
    }
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
    }
    free(window);
}

void window_poll_events(window_t *window) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void call_event(window_t *w, int event, void *data) {
    if (!w || !w->event_callback) return;
    w->event_callback(event, data);
}

static bool remap_mouse_coords(window_t *w, int win_x, int win_y, int *out_x, int *out_y) {
    if (!w || !w->fb) {
        *out_x = win_x;
        *out_y = win_y;
        return true;
    }

    int viewport_w = w->destRect.right - w->destRect.left;
    int viewport_h = w->destRect.bottom - w->destRect.top;

    if (win_x < w->destRect.left || win_x >= w->destRect.right ||
        win_y < w->destRect.top || win_y >= w->destRect.bottom) {
        return false;
    }

    if (viewport_w > 0 && viewport_h > 0) {
        float u = (float)(win_x - w->destRect.left) / viewport_w;
        float v = (float)(win_y - w->destRect.top) / viewport_h;
        *out_x = (int)(u * w->fb->width);
        *out_y = (int)(v * w->fb->height);
    } else {
        *out_x = 0;
        *out_y = 0;
    }

    return true;
}

static LRESULT CALLBACK platform_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    window_t *w = (window_t *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if (w) {
                window_blit(w);
            }
            EndPaint(hwnd, &ps);
        } break;

        case WM_SIZE: {
            if (w) {
                event_window_resize_t ev;
                ev.width = LOWORD(lParam);
                ev.height = HIWORD(lParam);
                call_event(w, EVENT_WINDOW_RESIZE, &ev);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        } break;
        case WM_CLOSE: {
            if (w) {
                call_event(w, EVENT_WINDOW_CLOSE, NULL);
            }
            DestroyWindow(hwnd);
        } break;
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;
        case WM_SETFOCUS: {
            if (w) call_event(w, EVENT_WINDOW_FOCUS_GAINED, NULL);
        } break;
        case WM_KILLFOCUS: {
            if (w) call_event(w, EVENT_WINDOW_FOCUS_LOST, NULL);
        } break;
        case WM_MOUSEMOVE: {
            if (w) {
                int win_x = GET_X_LPARAM(lParam);
                int win_y = GET_Y_LPARAM(lParam);
                event_mouse_move_t ev;

                bool in_viewport = remap_mouse_coords(w, win_x, win_y, &ev.x, &ev.y);

                if (in_viewport) {
                    call_event(w, EVENT_MOUSE_MOVE, &ev);
                    if (!w->mouse_inside) {
                        w->mouse_inside = true;
                        call_event(w, EVENT_MOUSE_ENTER, NULL);
                    }
                } else {
                    if (w->mouse_inside) {
                        w->mouse_inside = false;
                        call_event(w, EVENT_MOUSE_LEAVE, NULL);
                    }
                }
            }
        } break;
        case WM_MOUSELEAVE: {
            if (w) {
                if (w->mouse_inside) {
                    w->mouse_inside = false;
                    call_event(w, EVENT_MOUSE_LEAVE, NULL);
                }
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            if (w) {
                event_mouse_button_t ev;
                int win_x = GET_X_LPARAM(lParam);
                int win_y = GET_Y_LPARAM(lParam);
                remap_mouse_coords(w, win_x, win_y, &ev.x, &ev.y);
                ev.button = (msg == WM_LBUTTONDOWN) ? 1 : (msg == WM_RBUTTONDOWN) ? 2 : 3;
                call_event(w, EVENT_MOUSE_BUTTON_DOWN, &ev);
            }
        } break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            if (w) {
                event_mouse_button_t ev;
                int win_x = GET_X_LPARAM(lParam);
                int win_y = GET_Y_LPARAM(lParam);
                remap_mouse_coords(w, win_x, win_y, &ev.x, &ev.y);
                ev.button = (msg == WM_LBUTTONUP) ? 1 : (msg == WM_RBUTTONUP) ? 2 : 3;
                call_event(w, EVENT_MOUSE_BUTTON_UP, &ev);
            }
        } break;
        case WM_MOUSEWHEEL: {
            if (w) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &pt);
                event_mouse_wheel_t ev;
                remap_mouse_coords(w, pt.x, pt.y, &ev.x, &ev.y);
                int z = GET_WHEEL_DELTA_WPARAM(wParam);
                ev.dx = 0;
                ev.dy = z;
                call_event(w, EVENT_MOUSE_WHEEL, &ev);
            }
        } break;
        case WM_KEYDOWN: {
            if (w) {
                keycode_t code = vk_to_keycode(wParam);
                event_key_t ev;
                ev.code = code;
                call_event(w, EVENT_KEY_DOWN, &ev);
            }
        } break;
        case WM_KEYUP: {
            if (w) {
                keycode_t code = vk_to_keycode(wParam);
                event_key_t ev;
                ev.code = code;
                call_event(w, EVENT_KEY_UP, &ev);
            }
        } break;
        case WM_CHAR: {
            if (w) {
                // unicode character in wParam (utf-16 code unit); convert to utf-32 for
                // single event here we pass the wchar as an int inside event_key_t.code
                // (not ideal for full utf8), but platform.h expects EVENT_KEY_CHAR;
                // we'll pass the wide char as a 32-bit value
                uint32_t ch = (uint32_t)wParam;
                // reuse event_key_t to carry the char in .code (not ideal but keeps API
                // small).
                event_key_t ev;
                ev.code = (keycode_t)ch;
                call_event(w, EVENT_KEY_CHAR, &ev);
            }
        } break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

#endif // _WIN32