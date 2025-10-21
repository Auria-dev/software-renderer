#include "main.h"

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480

static bool running = false;
static float delta_time = 0.0f;
static vec2 mousepos;
static vec3 cam_pos = {0,0,-5};
static vec3 cam_rot = {0,0,0};

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

u32 shader_gouraud(const render_context *ctx, const interpolator_t *interps, float inv_w) {
    // Attribute 1 is texcoord (type FRAG_ATTR_VEC2)
    // const interpolator_t *uv_interp = &interps[1];
    // vec2 uv;
    // uv.x = uv_interp->v[0];
    // uv.y = uv_interp->v[1];
    // if (uv_interp->perspective_correct) {
    //     uv.x *= inv_w;
    //     uv.y *= inv_w;
    // }

    // // Attribute 0 is color (type FRAG_ATTR_U32)
    const interpolator_t *color_interp = &interps[0];

    float r = color_interp->v[0];
    float g = color_interp->v[1];
    float b = color_interp->v[2];

    if (color_interp->perspective_correct) {
        r *= inv_w;
        g *= inv_w;
        b *= inv_w;
    }

    u32 r_u32 = (u32)((r < 0) ? 0 : (r > 255) ? 255 : r);
    u32 g_u32 = (u32)((g < 0) ? 0 : (g > 255) ? 255 : g);
    u32 b_u32 = (u32)((b < 0) ? 0 : (b > 255) ? 255 : b);

    return 0xFF000000 | (r_u32 << 16) | (g_u32 << 8) | b_u32;
}

int main(int argc, char *argv[]) {
    window_t *win = window_create("Hello :D", RENDER_WIDTH, RENDER_HEIGHT, handle_event);
    if (!win) { fprintf(stderr, "Failed to create window\n"); return 1; }

    render_context ctx = render_context_init(
        RENDER_WIDTH, RENDER_HEIGHT,
        70.0f, (float)RENDER_WIDTH / (float)RENDER_HEIGHT, 0.1f, 100.0f,
        (vec3){0,0,5}, (vec3){0,0,0}, (vec3){0,1,0},
        true, false, false
    );
    
    window_bind_framebuffer(win, &ctx.framebuffer);
    g_bind_fragment_shader(&ctx, shader_gouraud);
    g_update_projection_matrix(&ctx, 70.0f, (float)ctx.framebuffer.height / (float)ctx.framebuffer.width);

    texture_manager_t texture_manager = tm_init();
    
    mesh_t mymodel = {0};
    load_obj("assets/models/monkey.obj", &mymodel, &texture_manager);

    mymodel.position = (vec3){0,0,0};
    mymodel.rotation = (vec3){0,0,0};
    mymodel.scale    = (vec3){1,1,1};

    running = true;
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    int frames = 0;

    while (running) {
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

        memset(ctx.framebuffer.color_buffer, (int)0xff222222, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(u32));
        memset(ctx.framebuffer.depth_buffer, 0.0f, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(float));

        // mymodel.rotation.y += 0.5f * delta_time;
        // mymodel.rotation.x += 0.1f * delta_time;

        // draw loaded model
        g_update_world_matrix(&ctx, mymodel.position, mymodel.rotation, mymodel.scale);
        for (int i = 0; i < mymodel.submesh_count; i++) {
            submesh_t *sm = &mymodel.submeshes[i];
            g_bind_material(&ctx, sm->material_id);
            g_bind_buffer(&ctx, GBUFFER_VERTEX, mymodel.vertices, mymodel.vertex_count * sizeof(vertex_t));
            g_bind_buffer(&ctx, GBUFFER_INDEX, sm->indices, sm->index_count * sizeof(u32));

            g_draw_elements(&ctx, sm->index_count, sm->indices);
        }

        window_blit(win);
        frames++;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        delta_time = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) * 1e-9;
        static double fps_timer = 0.0;
        fps_timer += delta_time;
        frames++;

        if (fps_timer >= 1.0) {
            char title[64];
            snprintf(title, 64, "software renderer - fps: %d", frames);
            window_set_title(win, title);
            fps_timer = 0.0;
            frames = 0;
        }

        last_time = now;
    }

    tm_free(&texture_manager);
    window_destroy(win);
    return 0;
}
