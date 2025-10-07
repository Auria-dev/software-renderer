#include "main.h"

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480

static bool running = false;
static float delta_time = 0.0f;
static vec2 mousepos;
static vec3 cam_pos = {0,0,-5};

struct movement {
    bool forward, backward;
    bool left, right;
    bool up, down;
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
    window_t *win = window_create("Hello :D", 640, 480, handle_event);
    if (!win) { fprintf(stderr, "Failed to create window\n"); return 1; }

    render_context ctx = render_context_init(
        RENDER_WIDTH, RENDER_HEIGHT,
        70.0f, (float)RENDER_WIDTH / (float)RENDER_HEIGHT, 0.1f, 100.0f,
        (vec3){0,0,5}, (vec3){0,0,0}, (vec3){0,1,0},
        true, false, true
    );
    window_bind_framebuffer(win, &ctx.framebuffer);

    // load model
    // mesh_t mymesh = load_mesh("assets/meshes/cube.obj", &ctx.vertex_buffer, &ctx.index_buffer); // also loads the MTL.
    
    // cube mesh hardcoded
    mesh_t mymesh = {0};
    mymesh.vertex_count = 8;
    mymesh.vertices = malloc(sizeof(vertex_t) * mymesh.vertex_count);
    mymesh.vertices[0].position = (vec3){-1,-1,-1};
    mymesh.vertices[1].position = (vec3){ 1,-1,-1};
    mymesh.vertices[2].position = (vec3){ 1, 1,-1};
    mymesh.vertices[3].position = (vec3){-1, 1,-1};
    mymesh.vertices[4].position = (vec3){-1,-1, 1};
    mymesh.vertices[5].position = (vec3){ 1,-1, 1};
    mymesh.vertices[6].position = (vec3){ 1, 1, 1};
    mymesh.vertices[7].position = (vec3){-1, 1, 1};
    for (int i = 0; i < mymesh.vertex_count; i++) {
        mymesh.vertices[i].normal = vec3_zero();
        mymesh.vertices[i].texcoord = (vec2){0,0};
    }
    mymesh.submesh_count = 1;
    mymesh.submeshes = malloc(sizeof(submesh_t) * mymesh.submesh_count);
    mymesh.submeshes[0].material_id = 0; // default material
    mymesh.submeshes[0].index_count = 36;
    mymesh.submeshes[0].indices = malloc(sizeof(u32) * mymesh.submeshes[0].index_count);
    u32 cube_indices[] = {
        0,1,2,  2,3,0,
        1,5,6,  6,2,1,
        5,4,7,  7,6,5,
        4,0,3,  3,7,4,
        3,2,6,  6,7,3,
        4,5,1,  1,0,4
    };
    memcpy(mymesh.submeshes[0].indices, cube_indices, sizeof(u32) * mymesh.submeshes[0].index_count);
    mymesh.position = (vec3){0,0,0};
    mymesh.rotation = (vec3){0,0,0};
    mymesh.scale    = (vec3){1,1,1};
    
    // setup render context matrices
    g_update_projection_matrix(&ctx, 70.0f, (float)ctx.framebuffer.height / (float)ctx.framebuffer.width);

    running = true;
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    int frames = 0;

    while (running) {
        window_poll_events(win);

        if (movement.forward)  cam_pos.z += 5.0f * delta_time;
        if (movement.backward) cam_pos.z -= 5.0f * delta_time;
        if (movement.left)     cam_pos.x += 5.0f * delta_time;
        if (movement.right)    cam_pos.x -= 5.0f * delta_time;
        if (movement.up)       cam_pos.y += 5.0f * delta_time;
        if (movement.down)     cam_pos.y -= 5.0f * delta_time;

        g_update_view_matrix(&ctx, mat4_look_at(cam_pos, vec3_sub(cam_pos, vec3_new(0,0,-1)), (vec3){0,1,0})); // up is just +Y cause no rotation yet
        g_update_world_matrix(&ctx, mymesh.position, mymesh.rotation, mymesh.scale);

        mymesh.rotation.y += 0.5f * delta_time;
        mymesh.rotation.x += 0.1f * delta_time;

        memset(ctx.framebuffer.color_buffer, (int)0xff222222, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(u32));

        // draw the mesh
        for (int i = 0; i < mymesh.submesh_count; i++) {
            submesh_t *sm = &mymesh.submeshes[i];
            g_bind_material(&ctx, sm->material_id);
            g_bind_buffer(&ctx, GBUFFER_VERTEX, mymesh.vertices, mymesh.vertex_count * sizeof(vertex_t));
            g_bind_buffer(&ctx, GBUFFER_INDEX, sm->indices, sm->index_count * sizeof(u32));

            g_draw_elements(&ctx, sm->index_count, sm->indices);
        }

        window_blit(win);
        frames++;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        // calculate delta time in seconds
        delta_time = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) * 1e-9;

        // update frame counter and title once per second
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

    window_destroy(win);
    return 0;
}