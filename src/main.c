#include "main.h"

#define RENDER_WIDTH 640/2
#define RENDER_HEIGHT 480/2

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
            if (e->code == KEY_ESCAPE) running = false;
        } break;
        case EVENT_KEY_UP: {
            event_key_t *e = data;
            printf("key up: %d\n", e->code);
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
        true, false, false
    );
    window_bind_framebuffer(win, &ctx.framebuffer);

    // load model
    // mesh_t mymesh = load_mesh("assets/meshes/cube.obj", &ctx.vertex_buffer, &ctx.index_buffer); // also loads the MTL.

    // setup render context matrices
    // g_update_projection_matrix(&ctx.projection_matrix, 70.0f, (float)ctx.framebuffer.width / (float)ctx.framebuffer.height, 0.1f, 100.0f);
    // g_update_view_matrix(&ctx.view_matrix, mat4_lookat((vec3){0,0,0}, (vec3){0,0,-1}, (vec3){0,1,0}));
    // g_update_model_matrix(&ctx.model_matrix, mymesh.position, mymesh.rotation, mymesh.scale);

    running = true;
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    int frames = 0;

    while (running) {
        window_poll_events(win);

        // TOOD: camera controls
        // g_update_view_matrix(&ctx, mat4_lookat((vec3){0,0,0}, (vec3){0,0,-1}, (vec3){0,1,0}));

        static int frame = 0;
        memset(ctx.framebuffer.color_buffer, (int)0xff222222, ctx.framebuffer.width * ctx.framebuffer.height * sizeof(u32));

        // update model matrix
        // g_update_model_matrix(&ctx.model_matrix, mymesh.position, mymesh.rotation, mymesh.scale);
        

        // draw the mesh
        // for (int i = 0; i < mymesh.submesh_count; i++) {
        //     submesh_t *sm = &mymesh.submeshes[i]
        //     g_bind_material(&ctx, sm->material_id);
        //     g_bind_buffer(&ctx, GBUFFER_VERTEX, ctx.vertex_buffer.data, ctx.vertex_buffer.size);
        //     g_bind_buffer(&ctx, GBUFFER_INDEX, ctx.index_buffer.data, ctx.index_buffer.size);
        //     
        //     g_draw_elements(&ctx, sm->index_count, sm->indices);
        // }

        draw_pixel(&ctx, mousepos.x, mousepos.y, 0xff00ffff);

        frame++;
        window_blit(win);
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