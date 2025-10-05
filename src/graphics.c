#include "graphics.h"

render_context render_context_init(
    int width, int height,
    float fov, float aspect_ratio, float near, float far,
    vec3 cam_pos, vec3 cam_target, vec3 cam_up,
    bool enable_depth_test, bool enable_blend_test, bool enable_cull_face) {
    render_context ctx = {0};
    ctx.projection_matrix = mat4_make_perspective(fov, aspect_ratio, near, far);
    ctx.view_matrix = mat4_look_at(cam_pos, cam_target, cam_up);
    ctx.world_matrix = mat4_identity();
    ctx.framebuffer = framebuffer_init(width, height);
    ctx.vertex_buffer.data = NULL;
    ctx.vertex_buffer.size = 0;
    ctx.vertex_buffer.type = GBUFFER_VERTEX;
    ctx.index_buffer.data = NULL;
    ctx.index_buffer.size = 0;
    ctx.index_buffer.type = GBUFFER_INDEX;
    ctx.texture_manager = tm_init();
    ctx.clip_near = near;
    ctx.clip_far = far;
    ctx.depth_test = enable_depth_test;
    ctx.blend_test = enable_blend_test;
    ctx.cull_face = enable_cull_face;
    ctx.material_id = -1;
    return ctx;
}

framebuffer_t framebuffer_init(int width, int height) {
    framebuffer_t fb;
    fb.width = width;
    fb.height = height;
    fb.color_buffer = calloc(fb.width * fb.height, sizeof(u32));
    fb.depth_buffer = calloc(fb.width * fb.height, sizeof(float));
    return fb;
}

void draw_pixel(render_context *ctx, int x, int y, u32 c) {
    if (x < 0 || x >= ctx->framebuffer.width || y < 0 || y >= ctx->framebuffer.height) return;
    ctx->framebuffer.color_buffer[y * ctx->framebuffer.width + x] = c;
}

void g_bind_material(render_context *ctx, int material_id) {
    // TOOD: sanity checks
    ctx->material_id = material_id;
}

void g_bind_buffer(render_context *ctx, u32 type, void *data, size_t size) {
    
}

void g_buffer_data(render_context *ctx, u32 type, void *data, size_t size) {

}

void g_draw_elements(render_context *ctx, u32 count, u32 *indices) {
    // this will do everything from projecting the points to clipping to rasterizing
    // apply all trasformations to each vertex
    // projected the points
    // split into triangles
    // clip the triangles
    // rasterize the triangles
}