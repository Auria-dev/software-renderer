#include "main.h"

// #define RENDER_WIDTH 640
// #define RENDER_HEIGHT 480

#define RENDER_WIDTH 1920
#define RENDER_HEIGHT 1080

// #define SAMPLE_BILINEAR

static bool running = false;
static float delta_time = 0.0f;
static vec2 mousepos;
static vec3 cam_pos = {0,0,-1};
static vec3 cam_rot = {0,0,0};
static bool mouse_captured = false;

struct movement {
    bool forward, backward;
    bool left, right;
    bool up, down;

    bool llook, rlook;
    bool ulook, dlook;
} movement = {0};

void handle_event(int event, void *data) {
    switch (event) {
        case EVENT_WINDOW_CLOSE: {
            running = false;
        } break;
        case EVENT_KEY_DOWN: {
            event_key_t *e = data;
            if (e->code == KEY_ESCAPE) running = false;
            switch (e->code) {
                case KEY_W:     movement.forward  = true; break;
                case KEY_S:     movement.backward = true; break;
                case KEY_A:     movement.left     = true; break;
                case KEY_D:     movement.right    = true; break;
                case KEY_SPACE: movement.up       = true; break;
                case KEY_SHIFT: movement.down     = true; break;

                // arrow keys look
                case KEY_UP:    movement.ulook    = true; break;
                case KEY_DOWN:  movement.dlook    = true; break;
                case KEY_LEFT:  movement.llook    = true; break;
                case KEY_RIGHT: movement.rlook    = true; break;
                default: break;
            }
        } break;
        case EVENT_KEY_UP: {
            event_key_t *e = data;
            switch (e->code) {
                case KEY_W:     movement.forward  = false; break;
                case KEY_S:     movement.backward = false; break;
                case KEY_A:     movement.left     = false; break;
                case KEY_D:     movement.right    = false; break;
                case KEY_SPACE: movement.up       = false; break;
                case KEY_SHIFT: movement.down     = false; break;

                // arrow keys look
                case KEY_UP:    movement.ulook    = false; break;
                case KEY_DOWN:  movement.dlook    = false; break;
                case KEY_LEFT:  movement.llook    = false; break;
                case KEY_RIGHT: movement.rlook    = false; break;

                case KEY_ENTER:
                    mouse_captured = !mouse_captured;
                    if (mouse_captured) {
                        window_set_mouse_position(NULL, RENDER_WIDTH / 2, RENDER_HEIGHT / 2);
                        window_show_cursor(NULL, mouse_captured);
                    }
                    break;
                default: break;
            }
        } break;
        case EVENT_MOUSE_MOVE: {
            event_mouse_move_t *e = data;
            mousepos.x = e->x;
            mousepos.y = e->y;
        } break;
    }
}

int main(int argc, char *argv[]) {
    window_t *win = window_create("Hello :D", RENDER_WIDTH, RENDER_HEIGHT, handle_event);
    if (!win) { fprintf(stderr, "Failed to create window\n"); return 1; }

    render_context ctx = render_context_init(
        RENDER_WIDTH, RENDER_HEIGHT,
        70.0f, (float)RENDER_WIDTH / (float)RENDER_HEIGHT, 0.1f, 1000.0f,
        (vec3){0,0,5}, (vec3){0,0,0}, (vec3){0,1,0},
        true, false, true
    );
    window_bind_framebuffer(win, &ctx.framebuffer);

    mesh_t camera = {0};
    load_obj("assets/models/Camera_01_1k.obj", &camera, ctx.material_manager);
    camera.position = (vec3){0,-1,0};
    camera.rotation = (vec3){0,32,0};
    camera.scale    = (vec3){10,10,10};

    mesh_t floor = {0};
    load_obj("assets/models/dirt_1k.obj", &floor, ctx.material_manager);
    floor.position = (vec3){0,-1,0};
    floor.rotation = (vec3){0,0,0};
    floor.scale    = (vec3){1,1,1};

    // setup render context matrices
    g_update_projection_matrix(&ctx, 70.0f, (float)ctx.framebuffer.height / (float)ctx.framebuffer.width);

    double last_frame_time = (double)clock() / CLOCKS_PER_SEC;
    double fps_timer = 0.0;
    int frame_count = 0;
    int last_fps = 0;

    running = true;
    while (running) {
        double current_time = (double)clock() / CLOCKS_PER_SEC; 
        delta_time = (float)(current_time - last_frame_time);
        last_frame_time = current_time;

        window_poll_events(win);
        if (movement.dlook)    cam_rot.x += 2.0f * delta_time;
        if (movement.ulook)    cam_rot.x -= 2.0f * delta_time;
        if (movement.llook)    cam_rot.y += 2.0f * delta_time;
        if (movement.rlook)    cam_rot.y -= 2.0f * delta_time;

        vec3 forward = { cosf(cam_rot.y-deg_to_rad(90)), 0, -sinf(cam_rot.y-deg_to_rad(90)) };
        vec3 right = {  sinf(cam_rot.y-deg_to_rad(90)), 0, cosf(cam_rot.y-deg_to_rad(90)) };
        forward = vec3_normalize(forward);
        right = vec3_normalize(right);
        if (movement.forward)  cam_pos = vec3_add(cam_pos, vec3_scale(forward, 5.0f * delta_time));
        if (movement.backward) cam_pos = vec3_sub(cam_pos, vec3_scale(forward, 5.0f * delta_time));
        if (movement.left)     cam_pos = vec3_sub(cam_pos, vec3_scale(right, 5.0f * delta_time));
        if (movement.right)    cam_pos = vec3_add(cam_pos, vec3_scale(right, 5.0f * delta_time));
        if (movement.up)       cam_pos.y += 5.0f * delta_time;
        if (movement.down)     cam_pos.y -= 5.0f * delta_time;

        vec3 target = vec3_forward();
        mat4 cam_rot_x = mat4_make_rotation_x(cam_rot.x);
        mat4 cam_rot_y = mat4_make_rotation_y(cam_rot.y);
        mat4 cam_rot_z = mat4_make_rotation_z(cam_rot.z);
        mat4 rotation_matrix = mat4_mul_mat4(mat4_mul_mat4(cam_rot_y, cam_rot_x), cam_rot_z);
        vec3 cam_dir = vec4_to_vec3(mat4_mul_vec4(rotation_matrix, vec3_to_vec4(target)));
        target = vec3_add(cam_pos, cam_dir);
        vec3 up = vec4_to_vec3(mat4_mul_vec4(rotation_matrix, vec3_to_vec4(vec3_up())));
        g_update_view_matrix(&ctx, mat4_look_at(cam_pos, target, up));

        memset(ctx.framebuffer.color_buffer, (int)0xff292d35, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(u32));
        memset(ctx.framebuffer.depth_buffer, 0.0f, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(float));

        g_set_bilinear_sampling(&ctx, true);

        g_draw_mesh(&ctx, &floor, MESH_GOURAUD);
        g_draw_mesh(&ctx, &camera, MESH_GOURAUD);

        window_blit(win);
        frame_count++;
        fps_timer += delta_time;

        if (fps_timer >= 1.0) {
            last_fps = frame_count;
            frame_count = 0;
            fps_timer -= 1.0;

            char title[64];
            snprintf(title, 64, "software renderer - fps: %d", last_fps);
            window_set_title(win, title);
        }

        // print cam pos/rot
        // printf("cam pos: %.2f, %.2f, %.2f | rot: %.2f, %.2f, %.2f\r", cam_pos.x, cam_pos.y, cam_pos.z, cam_rot.x, cam_rot.y, cam_rot.z);
    }

    m_free(ctx.material_manager);
    window_destroy(win);
    return 0;
}
