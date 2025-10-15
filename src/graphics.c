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
    ctx.frustum = frustum_init(fov, aspect_ratio, near, far);
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

#include <stdio.h>
frustum_t frustum_init(float fov, float aspect_ratio, float clipping_near, float clipping_far) {
    frustum_t frustum;
    clipping_plane_t *frustum_planes = frustum.planes;

    float fovy = deg_to_rad(fov);
    float fovx = atan(tan(fovy/2.0)*aspect_ratio) * 2;

    float cos_half_fov_x = cos(fovx/2);
    float sin_half_fov_x = sin(fovx/2);
    float cos_half_fov_y = cos(fovy/2);
    float sin_half_fov_y = sin(fovy/2);

    frustum_planes[FRUSTUM_LEFT].point = vec3_zero();
    frustum_planes[FRUSTUM_LEFT].normal.x = cos_half_fov_x;
    frustum_planes[FRUSTUM_LEFT].normal.y = 0;
    frustum_planes[FRUSTUM_LEFT].normal.z = sin_half_fov_x;

    frustum_planes[FRUSTUM_RIGHT].point = vec3_zero();
    frustum_planes[FRUSTUM_RIGHT].normal.x = -cos_half_fov_x;
    frustum_planes[FRUSTUM_RIGHT].normal.y = 0;
    frustum_planes[FRUSTUM_RIGHT].normal.z = sin_half_fov_x;

    frustum_planes[FRUSTUM_TOP].point = vec3_zero();
    frustum_planes[FRUSTUM_TOP].normal.x = 0;
    frustum_planes[FRUSTUM_TOP].normal.y = -cos_half_fov_y;
    frustum_planes[FRUSTUM_TOP].normal.z = sin_half_fov_y;
    
    frustum_planes[FRUSTUM_BOTTOM].point = vec3_zero();
    frustum_planes[FRUSTUM_BOTTOM].normal.x = 0;
    frustum_planes[FRUSTUM_BOTTOM].normal.y = cos_half_fov_y;
    frustum_planes[FRUSTUM_BOTTOM].normal.z = sin_half_fov_y;
    
    frustum_planes[FRUSTUM_NEAR].point = vec3_new(0,0,clipping_near);
    frustum_planes[FRUSTUM_NEAR].normal = vec3_new(0,0,1);
    frustum_planes[FRUSTUM_FAR].point = vec3_new(0,0,clipping_far);
    frustum_planes[FRUSTUM_FAR].normal = vec3_new(0,0,-1);

    return frustum;
}

void draw_pixel(render_context *ctx, int x, int y, u32 c) {
    if (x < 0 || x >= ctx->framebuffer.width || y < 0 || y >= ctx->framebuffer.height) return;
    ctx->framebuffer.color_buffer[y * ctx->framebuffer.width + x] = c;
}

void draw_line(render_context *ctx, int x0, int y0, int x1, int y1, u32 color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int current_x = x0;
    int current_y = y0;
    int err = dx - dy;

    while (true) {
        if (current_x >= 0 && current_x < ctx->framebuffer.width &&
            current_y >= 0 && current_y < ctx->framebuffer.height) {
            draw_pixel(ctx, current_x, current_y, color);
        }
        
        if (current_x == x1 && current_y == y1) break;

        int e2 = 2 * (int64_t)err;
        if (e2 > -dy) {
            err -= (int64_t)dy;
            current_x += sx;
        }
        if (e2 < dx) {
            err += (int64_t)dx;
            current_y += sy;
        }
    }
}

void g_update_projection_matrix(render_context *ctx, float fov, float ar) {
    ctx->projection_matrix = mat4_make_perspective(deg_to_rad(fov), ar, ctx->clip_near, ctx->clip_far);
}

void g_update_view_matrix(render_context *ctx, mat4 view) {
    ctx->view_matrix = view;
}

void g_update_world_matrix(render_context *ctx, vec3 position, vec3 rotation, vec3 scale) {
    mat4 scale_matrix = mat4_make_scale(scale.x, scale.y, scale.z);
    mat4 rotation_matrix_x = mat4_make_rotation_x(rotation.x);
    mat4 rotation_matrix_y = mat4_make_rotation_y(rotation.y);
    mat4 rotation_matrix_z = mat4_make_rotation_z(rotation.z);
    mat4 position_matrix = mat4_make_translation(position.x, position.y, position.z);

    mat4 world_matrix = mat4_identity();
    world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
    world_matrix = mat4_mul_mat4(position_matrix, world_matrix);

    ctx->world_matrix = world_matrix;
}

void g_bind_material(render_context *ctx, int material_id) {
    // TOOD: sanity checks
    ctx->material_id = material_id;
}

void g_bind_buffer(render_context *ctx, u32 type, void *data, size_t size) {
    switch (type) {
        case GBUFFER_INDEX: {
            ctx->index_buffer.data = data;
            ctx->index_buffer.size = size;
            ctx->index_buffer.type = type;
        } break;

        case GBUFFER_VERTEX: {
            ctx->vertex_buffer.data = data;
            ctx->vertex_buffer.size = size;
            ctx->vertex_buffer.type = type;
        } break;
        
        default:
            break;
    }
}

void g_draw_elements(render_context *ctx, u32 count, u32 *indices) {
    for (u32 i = 0; i < count; i += 3) {
        u32 vi0 = indices[i+0];
        u32 vi1 = indices[i+1];
        u32 vi2 = indices[i+2];

        vertex_t v0 = ((vertex_t*)ctx->vertex_buffer.data)[vi0];
        vertex_t v1 = ((vertex_t*)ctx->vertex_buffer.data)[vi1];
        vertex_t v2 = ((vertex_t*)ctx->vertex_buffer.data)[vi2];

        // Apply world and view transformations
        mat4 transform = mat4_mul_mat4(ctx->view_matrix, ctx->world_matrix);
        v0.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v0.position)));
        v1.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v1.position)));
        v2.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v2.position)));

        if (ctx->cull_face) {
            vec3 a = vec3_sub(v1.position, v0.position);
            vec3 b = vec3_sub(v2.position, v0.position);
            vec3 normal = vec3_normalize(vec3_cross(a, b));
            vec3 view_dir = vec3_normalize(v0.position);
            if (vec3_dot(normal, view_dir) > 0) {
                continue;
            }
        }

        vertex_t polygon_vertices[MAX_POLYGON_VERTICES] = {v0, v1, v2};
        int num_vertices = 3;

        for (int p = 0; p < 6; p++) {
            vertex_t clipped_vertices[MAX_POLYGON_VERTICES];
            num_vertices = clip_polygon_against_plane(polygon_vertices, num_vertices, clipped_vertices, &ctx->frustum.planes[p]);
            for (int j = 0; j < num_vertices; j++) {
                polygon_vertices[j] = clipped_vertices[j];
            }
        }

        if (num_vertices < 3) continue;

        for (int j = 1; j < num_vertices - 1; j++) {
            vertex_t tv0 = polygon_vertices[0];
            vertex_t tv1 = polygon_vertices[j];
            vertex_t tv2 = polygon_vertices[j + 1];

            vec4 pv0 = mat4_mul_vec4_project(ctx->projection_matrix, vec3_to_vec4(tv0.position));
            vec4 pv1 = mat4_mul_vec4_project(ctx->projection_matrix, vec3_to_vec4(tv1.position));
            vec4 pv2 = mat4_mul_vec4_project(ctx->projection_matrix, vec3_to_vec4(tv2.position));

            pv0.x *= -1.0f; pv1.x *= -1.0f; pv2.x *= -1.0f;
            pv0.y *= -1.0f; pv1.y *= -1.0f; pv2.y *= -1.0f;

            float screen0_x = (pv0.x * (ctx->framebuffer.width  / 2.0f)) + (ctx->framebuffer.width  / 2.0f);
            float screen0_y = (pv0.y * (ctx->framebuffer.height / 2.0f)) + (ctx->framebuffer.height / 2.0f);

            float screen1_x = (pv1.x * (ctx->framebuffer.width  / 2.0f)) + (ctx->framebuffer.width  / 2.0f);
            float screen1_y = (pv1.y * (ctx->framebuffer.height / 2.0f)) + (ctx->framebuffer.height / 2.0f);

            float screen2_x = (pv2.x * (ctx->framebuffer.width  / 2.0f)) + (ctx->framebuffer.width  / 2.0f);
            float screen2_y = (pv2.y * (ctx->framebuffer.height / 2.0f)) + (ctx->framebuffer.height / 2.0f);

            u32 color = ctx->material_id == 0 ? 0xffff0000 : 0xff00ff00;
            draw_line(ctx, (int)screen0_x, (int)screen0_y, (int)screen1_x, (int)screen1_y, color);
            draw_line(ctx, (int)screen1_x, (int)screen1_y, (int)screen2_x, (int)screen2_y, color);
            draw_line(ctx, (int)screen2_x, (int)screen2_y, (int)screen0_x, (int)screen0_y, color);
        }
    }
}
