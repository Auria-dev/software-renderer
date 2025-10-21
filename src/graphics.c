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
    ctx.frustum = frustum_init(fov, aspect_ratio, near, far);
    ctx.fragment_shader = NULL;
    
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

void draw_line_depth(render_context *ctx, vec3 p0, vec3 p1, uint32_t color) {
    const int dx = abs(p1.x - p0.x);
    const int dy = abs(p1.y - p0.y);
    const int sx = p0.x < p1.x ? 1 : -1;
    const int sy = p0.y < p1.y ? 1 : -1;
    const int steps = dx > dy ? dx : dy;
    
    const float rw0 = 1.0f / p0.z;
    const float rw1 = 1.0f / p1.z;
    float current_rw = rw0;
    const float rw_step = (steps > 0) ? ((rw1 - rw0) * (1.0f / steps)) : 0.0f;
    
    const float EPSILON_OFFSET = 20.0f * EPS;
    const int index_step_y = sy * ctx->framebuffer.width;
    
    int64_t dx64 = dx;
    int64_t dy64 = dy;
    int64_t err = dx64 - dy64;
    int current_x = p0.x;
    int current_y = p0.y;
    int index = current_y * ctx->framebuffer.width + current_x;

    for (int i = 0; i <= steps; ++i) {
        if ((unsigned)current_x < (unsigned)ctx->framebuffer.width && 
            (unsigned)current_y < (unsigned)ctx->framebuffer.height) {
            if (current_rw + EPSILON_OFFSET >= ctx->framebuffer.depth_buffer[index]) {
                ctx->framebuffer.depth_buffer[index] = current_rw;
                draw_pixel(ctx, current_x, current_y, color);
            }
        }

        if (i == steps) break;

        const int64_t e2 = err * 2;
        if (e2 > -dy64) {
            err -= dy64;
            current_x += sx;
            index += sx;
        }
        if (e2 < dx64) {
            err += dx64;
            current_y += sy;
            index += index_step_y;
        }
        current_rw += rw_step;
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

void g_bind_fragment_shader(render_context *ctx, fragment_shader_fn shader) {
    if (ctx) {
        ctx->fragment_shader = shader;
    }
}

        // fragment_attribute_t attrs0[2];
        // attrs0[0] = (fragment_attribute_t){ .data = &v_in0.color, .type = FRAG_ATTR_U32, .perspective_correct = true };
        // attrs0[1] = (fragment_attribute_t){ .data = &v_in0.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
        // vertex_attributes_t v_attr0 = { .attributes = attrs0, .attribute_count = 2 };

        // fragment_attribute_t attrs1[2];
        // attrs1[0] = (fragment_attribute_t){ .data = &v_in1.color, .type = FRAG_ATTR_U32, .perspective_correct = true };
        // attrs1[1] = (fragment_attribute_t){ .data = &v_in1.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
        // vertex_attributes_t v_attr1 = { .attributes = attrs1, .attribute_count = 2 };

        // fragment_attribute_t attrs2[2];
        // attrs2[0] = (fragment_attribute_t){ .data = &v_in2.color, .type = FRAG_ATTR_U32, .perspective_correct = true };
        // attrs2[1] = (fragment_attribute_t){ .data = &v_in2.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
        // vertex_attributes_t v_attr2 = { .attributes = attrs2, .attribute_count = 2 };
        
        // // Perspective divide
        // float inv_w0 = 1.0f / p0.w;
        // float inv_w1 = 1.0f / p1.w;
        // float inv_w2 = 1.0f / p2.w;

        // vec3 ndc0 = { p0.x * inv_w0, p0.y * inv_w0, p0.z * inv_w0 };
        // vec3 ndc1 = { p1.x * inv_w1, p1.y * inv_w1, p1.z * inv_w1 };
        // vec3 ndc2 = { p2.x * inv_w2, p2.y * inv_w2, p2.z * inv_w2 };

        // float x0 = (ndc0.x + 1.0f) * 0.5f * ctx->framebuffer.width;
        // float y0 = (1.0f - ndc0.y) * 0.5f * ctx->framebuffer.height;
        // float x1 = (ndc1.x + 1.0f) * 0.5f * ctx->framebuffer.width;
        // float y1 = (1.0f - ndc1.y) * 0.5f * ctx->framebuffer.height;
        // float x2 = (ndc2.x + 1.0f) * 0.5f * ctx->framebuffer.width;
        // float y2 = (1.0f - ndc2.y) * 0.5f * ctx->framebuffer.height;

        // draw_triangle_modular(ctx, 
        //     x0, y0, p0.w,
        //     x1, y1, p1.w,
        //     x2, y2, p2.w,
        //     &v_attr0, &v_attr1, &v_attr2
        // );


void g_draw_elements(render_context *ctx, u32 count, u32 *indices) {
    mat4 transform = mat4_mul_mat4(ctx->view_matrix, ctx->world_matrix);
    for (u32 i = 0; i < count; i += 3) {
        u32 vi0 = indices[i+0];
        u32 vi1 = indices[i+1];
        u32 vi2 = indices[i+2];

        vertex_t v0 = ((vertex_t*)ctx->vertex_buffer.data)[vi0];
        vertex_t v1 = ((vertex_t*)ctx->vertex_buffer.data)[vi1];
        vertex_t v2 = ((vertex_t*)ctx->vertex_buffer.data)[vi2];

        // Apply world and view transformations
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

            u32 color1 = 0xffff0000;
            u32 color2 = 0xff00ff00;
            u32 color3 = 0xff0000ff;

            // draw_line(ctx, (int)screen0_x, (int)screen0_y, (int)screen1_x, (int)screen1_y, color);
            // draw_line(ctx, (int)screen1_x, (int)screen1_y, (int)screen2_x, (int)screen2_y, color);
            // draw_line(ctx, (int)screen2_x, (int)screen2_y, (int)screen0_x, (int)screen0_y, color);
            
            // draw_triangle_scalar(ctx,
            //     screen0_x, screen0_y, pv0.w, tv0.texcoord.x, tv0.texcoord.y, color1,
            //     screen1_x, screen1_y, pv1.w, tv1.texcoord.x, tv1.texcoord.y, color2,
            //     screen2_x, screen2_y, pv2.w, tv2.texcoord.x, tv2.texcoord.y, color3
            // );

            fragment_attribute_t attrs0[2];
            attrs0[0] = (fragment_attribute_t){ .data = &color1, .type = FRAG_ATTR_U32, .perspective_correct = true };
            attrs0[1] = (fragment_attribute_t){ .data = &tv0.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
            vertex_attributes_t v_attr0 = { .attributes = attrs0, .attribute_count = 2 };

            fragment_attribute_t attrs1[2];
            attrs1[0] = (fragment_attribute_t){ .data = &color2, .type = FRAG_ATTR_U32, .perspective_correct = true };
            attrs1[1] = (fragment_attribute_t){ .data = &tv1.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
            vertex_attributes_t v_attr1 = { .attributes = attrs1, .attribute_count = 2 };

            fragment_attribute_t attrs2[2];
            attrs2[0] = (fragment_attribute_t){ .data = &color3, .type = FRAG_ATTR_U32, .perspective_correct = true };
            attrs2[1] = (fragment_attribute_t){ .data = &tv2.texcoord, .type = FRAG_ATTR_VEC2, .perspective_correct = true };
            vertex_attributes_t v_attr2 = { .attributes = attrs2, .attribute_count = 2 };
            
            draw_triangle_modular(ctx, 
                screen0_x, screen0_y, pv0.w,
                screen1_x, screen1_y, pv1.w,
                screen2_x, screen2_y, pv2.w,
                &v_attr0, &v_attr1, &v_attr2
            );
        }
    }
}

void draw_triangle_scalar(
        render_context *ctx,
        float x0, float y0, float w0, float u0, float v0, u32 c0,
        float x1, float y1, float w1, float u1, float v1, u32 c1,
        float x2, float y2, float w2, float u2, float v2, u32 c2) {
    float area = ((x2 - x0) * (y1 - y0)) - ((y2 - y0) * (x1 - x0));
    if (area < 0) {
        area = -area;
        swap(x1,x2);
        swap(y1,y2);
        swap(w1,w2);
        swap(u1,u2);
        swap(v1,v2);
        swap(c1,c2);
    }

    if (fabsf(area) < 1e-6f) return;

    const int win_width = ctx->framebuffer.width;
    const int win_height = ctx->framebuffer.height;

    const int min_x_f = (int) floorf(fmin(fmin(x0, x1), x2));
    const int min_y_f = (int) floorf(fmin(fmin(y0, y1), y2));
    const int max_x_f = (int)  ceilf(fmax(fmax(x0, x1), x2));
    const int max_y_f = (int)  ceilf(fmax(fmax(y0, y1), y2));

    const int clamped_min_x = (min_x_f < 0) ? 0 : min_x_f;
    const int clamped_min_y = (min_y_f < 0) ? 0 : min_y_f;
    const int clamped_max_x = (max_x_f >= win_width) ? (win_width - 1) : max_x_f;
    const int clamped_max_y = (max_y_f >= win_height) ? (win_height - 1) : max_y_f;

    if (clamped_min_x > clamped_max_x || clamped_min_y > clamped_max_y) return;

    const float r0 = (float)((c0 >> 16) & 0xFF);
    const float g0 = (float)((c0 >>  8) & 0xFF);
    const float b0 = (float)( c0        & 0xFF);
    const float r1 = (float)((c1 >> 16) & 0xFF);
    const float g1 = (float)((c1 >>  8) & 0xFF);
    const float b1 = (float)( c1        & 0xFF);
    const float r2 = (float)((c2 >> 16) & 0xFF);
    const float g2 = (float)((c2 >>  8) & 0xFF);
    const float b2 = (float)( c2        & 0xFF);

    const float rcp_area = 1.0f / area;
    const float rcp_w0 = 1.0f / w0;
    const float rcp_w1 = 1.0f / w1;
    const float rcp_w2 = 1.0f / w2;
    
    const float dx0 = y2 - y1;
    const float dy0 = x1 - x2;
    const float dx1 = y0 - y2;
    const float dy1 = x2 - x0;
    const float dx2 = y1 - y0;
    const float dy2 = x0 - x1;
    
    const float r0_persp = r0 * rcp_w0;
    const float g0_persp = g0 * rcp_w0;
    const float b0_persp = b0 * rcp_w0;
    const float r1_persp = r1 * rcp_w1;
    const float g1_persp = g1 * rcp_w1;
    const float b1_persp = b1 * rcp_w1;
    const float r2_persp = r2 * rcp_w2;
    const float g2_persp = g2 * rcp_w2;
    const float b2_persp = b2 * rcp_w2;
    
    // const float u0_persp = u0 * rcp_w0;
    // const float u1_persp = u1 * rcp_w1;
    // const float u2_persp = u2 * rcp_w2;
    // const float v0_persp = v0 * rcp_w0;
    // const float v1_persp = v1 * rcp_w1;
    // const float v2_persp = v2 * rcp_w2;

    const float depth_dx = rcp_area * (rcp_w0 * dx0 + rcp_w1 * dx1 + rcp_w2 * dx2);
    const float depth_dy = rcp_area * (rcp_w0 * dy0 + rcp_w1 * dy1 + rcp_w2 * dy2);
    // const float u_dx = rcp_area * (u0_persp * dx0 + u1_persp * dx1 + u2_persp * dx2);
    // const float u_dy = rcp_area * (u0_persp * dy0 + u1_persp * dy1 + u2_persp * dy2);
    // const float v_dx = rcp_area * (v0_persp * dx0 + v1_persp * dx1 + v2_persp * dx2);
    // const float v_dy = rcp_area * (v0_persp * dy0 + v1_persp * dy1 + v2_persp * dy2);
    const float r_dx = rcp_area * (r0_persp * dx0 + r1_persp * dx1 + r2_persp * dx2);
    const float g_dx = rcp_area * (g0_persp * dx0 + g1_persp * dx1 + g2_persp * dx2);
    const float b_dx = rcp_area * (b0_persp * dx0 + b1_persp * dx1 + b2_persp * dx2);
    const float r_dy = rcp_area * (r0_persp * dy0 + r1_persp * dy1 + r2_persp * dy2);
    const float g_dy = rcp_area * (g0_persp * dy0 + g1_persp * dy1 + g2_persp * dy2);
    const float b_dy = rcp_area * (b0_persp * dy0 + b1_persp * dy1 + b2_persp * dy2);
   
    const float psx = clamped_min_x+0.5f;
    const float psy = clamped_min_y+0.5f;
    float w0_row = (psx - x1) * (y2 - y1) - (psy - y1) * (x2 - x1);
    float w1_row = (psx - x2) * (y0 - y2) - (psy - y2) * (x0 - x2);
    float w2_row = (psx - x0) * (y1 - y0) - (psy - y0) * (x1 - x0);
    float depth_recip_row = rcp_area * (rcp_w0 * w0_row + rcp_w1 * w1_row + rcp_w2 * w2_row);
    // float u_row = rcp_area * (u0_persp * w0_row + u1_persp * w1_row + u2_persp * w2_row);
    // float v_row = rcp_area * (v0_persp * w0_row + v1_persp * w1_row + v2_persp * w2_row);
    float r_row = rcp_area * (r0_persp * w0_row + r1_persp * w1_row + r2_persp * w2_row);
    float g_row = rcp_area * (g0_persp * w0_row + g1_persp * w1_row + g2_persp * w2_row);
    float b_row = rcp_area * (b0_persp * w0_row + b1_persp * w1_row + b2_persp * w2_row);

    intptr_t row_offset = clamped_min_y * win_width;
    for (int y = clamped_min_y; y <= clamped_max_y; ++y) {
        float* z_ptr = ctx->framebuffer.depth_buffer + row_offset + clamped_min_x;
        u32* color_ptr = ctx->framebuffer.color_buffer + row_offset + clamped_min_x;

        float w0_start = w0_row;
        float w1_start = w1_row;
        float w2_start = w2_row;
        // float u_start  = u_row;
        // float v_start  = v_row;
        float r_start  = r_row;
        float g_start  = g_row;
        float b_start  = b_row;
        float depth    = depth_recip_row;
        
        for (int x = clamped_min_x; x <= clamped_max_x; x++) {
            if (w0_start < 0 || w1_start < 0 || w2_start < 0 || depth < *z_ptr) {
                w0_start += dx0;
                w1_start += dx1;
                w2_start += dx2;
                // u_start  += u_dx;
                // v_start  += v_dx;
                r_start  += r_dx;
                g_start  += g_dx;
                b_start  += b_dx;
                depth    += depth_dx;
                z_ptr++;
                color_ptr++;
                continue;
            }

            const float inv_w = 1.0f / depth;
            // const float u = u_start * inv_w;
            // const float v = v_start * inv_w;
            const int  vr = r_start * inv_w;
            const int  vg = g_start * inv_w;
            const int  vb = b_start * inv_w;

            // const int tex_x = (int)(u * tex_width) & tex_width_mask;
            // const int tex_y = (int)(v * tex_height) & tex_height_mask;
            // const u8* texel = texture->data + (tex_y * tex_width + tex_x) * 4;

            // if (texel[3] == 0x00) {
            //     w0_start += dx0;
            //     w1_start += dx1;
            //     w2_start += dx2;
            //     u_start  += u_dx;
            //     v_start  += v_dx;
            //     r_start  += r_dx;
            //     g_start  += g_dx;
            //     b_start  += b_dx;
            //     depth    += depth_dx;
            //     z_ptr++;
            //     color_ptr++;
            //     continue;
            // }
            
            // const u8 tr = texel[0];
            // const u8 tg = texel[1];
            // const u8 tb = texel[2];
            
            const u8 tr = 0xff;
            const u8 tg = 0xff;
            const u8 tb = 0xff;

            int mod_r = (tr*vr)>>8;
            int mod_g = (tg*vg)>>8;
            int mod_b = (tb*vb)>>8;
            
            mod_r = (mod_r < 0) ? 0 : (mod_r > 255) ? 255 : mod_r;
            mod_g = (mod_g < 0) ? 0 : (mod_g > 255) ? 255 : mod_g;
            mod_b = (mod_b < 0) ? 0 : (mod_b > 255) ? 255 : mod_b;
            
            *z_ptr = depth;
            *color_ptr = 0xffu << 24 | mod_r << 16 | mod_g << 8 | mod_b;
                        
            w0_start += dx0;
            w1_start += dx1;
            w2_start += dx2;
            // u_start  += u_dx;
            // v_start  += v_dx;
            r_start  += r_dx;
            g_start  += g_dx;
            b_start  += b_dx;
            depth    += depth_dx;

            z_ptr++;
            color_ptr++;
        }

        w0_row += dy0;
        w1_row += dy1;
        w2_row += dy2;
        // u_row += u_dy;
        // v_row += v_dy;
        r_row += r_dy;
        g_row += g_dy;
        b_row += b_dy;
        depth_recip_row += depth_dy;
        row_offset += win_width;
    }
}

void draw_triangle_modular(render_context *ctx, 
    float x0, float y0, float w0,
    float x1, float y1, float w1,
    float x2, float y2, float w2,
    vertex_attributes_t *v0, vertex_attributes_t *v1, vertex_attributes_t *v2) {

    float area = ((x2 - x0) * (y1 - y0)) - ((y2 - y0) * (x1 - x0));
    if (area < 0) {
        area = -area;
        swap(x1,x2);
        swap(y1,y2);
        swap(w1,w2);
        swap(v1,v2);
    }

    if (fabsf(area) < 1e-6f) return;

    const int win_width = ctx->framebuffer.width;
    const int win_height = ctx->framebuffer.height;

    const int min_x_f = (int) floorf(fmin(fmin(x0, x1), x2));
    const int min_y_f = (int) floorf(fmin(fmin(y0, y1), y2));
    const int max_x_f = (int)  ceilf(fmax(fmax(x0, x1), x2));
    const int max_y_f = (int)  ceilf(fmax(fmax(y0, y1), y2));

    const int clamped_min_x = (min_x_f < 0) ? 0 : min_x_f;
    const int clamped_min_y = (min_y_f < 0) ? 0 : min_y_f;
    const int clamped_max_x = (max_x_f >= win_width) ? (win_width - 1) : max_x_f;
    const int clamped_max_y = (max_y_f >= win_height) ? (win_height - 1) : max_y_f;

    if (clamped_min_x > clamped_max_x || clamped_min_y > clamped_max_y) return;

    const float rcp_area = 1.0f / area;
    const float rcp_w0 = 1.0f / w0;
    const float rcp_w1 = 1.0f / w1;
    const float rcp_w2 = 1.0f / w2;
    
    const float dx0 = y2 - y1;
    const float dy0 = x1 - x2;
    const float dx1 = y0 - y2;
    const float dy1 = x2 - x0;
    const float dx2 = y1 - y0;
    const float dy2 = x0 - x1;

    const float depth_dx = rcp_area * (rcp_w0 * dx0 + rcp_w1 * dx1 + rcp_w2 * dx2);
    const float depth_dy = rcp_area * (rcp_w0 * dy0 + rcp_w1 * dy1 + rcp_w2 * dy2);

    const float psx = clamped_min_x+0.5f;
    const float psy = clamped_min_y+0.5f;
    float w0_row = (psx - x1) * (y2 - y1) - (psy - y1) * (x2 - x1);
    float w1_row = (psx - x2) * (y0 - y2) - (psy - y2) * (x0 - x2);
    float w2_row = (psx - x0) * (y1 - y0) - (psy - y0) * (x1 - x0);
    float depth_recip_row = rcp_area * (rcp_w0 * w0_row + rcp_w1 * w1_row + rcp_w2 * w2_row);

    interpolator_t interpolators[v0->attribute_count];
    for (u32 attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
        interpolators[attr_idx] = (interpolator_t){0};
        interpolator_t *curr_v = &interpolators[attr_idx];
        fragment_attribute_t *attr0 = &v0->attributes[attr_idx];
        fragment_attribute_t *attr1 = &v1->attributes[attr_idx];
        fragment_attribute_t *attr2 = &v2->attributes[attr_idx];
        
        switch (attr0->type) {
            case FRAG_ATTR_FLOAT: {
                curr_v->component_count = 1;
                curr_v->perspective_correct = attr0->perspective_correct;

                float a0 = *(float*)attr0->data;
                float a1 = *(float*)attr1->data;
                float a2 = *(float*)attr2->data; 
                
                if (attr0->perspective_correct) {
                    a0 *= rcp_w0;
                    a1 *= rcp_w1;
                    a2 *= rcp_w2;
                } 

                curr_v->dx[0]  = rcp_area * (a0 * dx0    + a1 * dx1    + a2 * dx2   );
                curr_v->dy[0]  = rcp_area * (a0 * dy0    + a1 * dy1    + a2 * dy2   );
                curr_v->row[0] = rcp_area * (a0 * w0_row + a1 * w1_row + a2 * w2_row); 
            } break;
            
            case FRAG_ATTR_VEC2:  {
                curr_v->component_count = 2;
                curr_v->perspective_correct = attr0->perspective_correct;

                vec2 a0 = *(vec2*)attr0->data;
                vec2 a1 = *(vec2*)attr1->data;
                vec2 a2 = *(vec2*)attr2->data;

                if (attr0->perspective_correct) {
                    a0.x *= rcp_w0; a0.y *= rcp_w0;
                    a1.x *= rcp_w1; a1.y *= rcp_w1;
                    a2.x *= rcp_w2; a2.y *= rcp_w2;
                }

                curr_v->dx[0]  = rcp_area * (a0.x * dx0    + a1.x * dx1    + a2.x * dx2   );
                curr_v->dy[0]  = rcp_area * (a0.x * dy0    + a1.x * dy1    + a2.x * dy2   );
                curr_v->row[0] = rcp_area * (a0.x * w0_row + a1.x * w1_row + a2.x * w2_row);

                curr_v->dx[1]  = rcp_area * (a0.y * dx0    + a1.y * dx1    + a2.y * dx2   );
                curr_v->dy[1]  = rcp_area * (a0.y * dy0    + a1.y * dy1    + a2.y * dy2   );
                curr_v->row[1] = rcp_area * (a0.y * w0_row + a1.y * w1_row + a2.y * w2_row);
            } break;
            
            case FRAG_ATTR_VEC3:  {
                curr_v->component_count = 3;
                curr_v->perspective_correct = attr0->perspective_correct;

                vec3 a0 = *(vec3*)attr0->data;
                vec3 a1 = *(vec3*)attr1->data;
                vec3 a2 = *(vec3*)attr2->data;

                if (attr0->perspective_correct) {
                    a0.x *= rcp_w0; a0.y *= rcp_w0; a0.z *= rcp_w0;
                    a1.x *= rcp_w1; a1.y *= rcp_w1; a1.z *= rcp_w1;
                    a2.x *= rcp_w2; a2.y *= rcp_w2; a2.z *= rcp_w2;
                }

                curr_v->dx[0]  = rcp_area * (a0.x * dx0    + a1.x * dx1    + a2.x * dx2   );
                curr_v->dy[0]  = rcp_area * (a0.x * dy0    + a1.x * dy1    + a2.x * dy2   );
                curr_v->row[0] = rcp_area * (a0.x * w0_row + a1.x * w1_row + a2.x * w2_row);

                curr_v->dx[1]  = rcp_area * (a0.y * dx0    + a1.y * dx1    + a2.y * dx2   );
                curr_v->dy[1]  = rcp_area * (a0.y * dy0    + a1.y * dy1    + a2.y * dy2   );
                curr_v->row[1] = rcp_area * (a0.y * w0_row + a1.y * w1_row + a2.y * w2_row);

                curr_v->dx[2]  = rcp_area * (a0.z * dx0    + a1.z * dx1    + a2.z * dx2   );
                curr_v->dy[2]  = rcp_area * (a0.z * dy0    + a1.z * dy1    + a2.z * dy2   );
                curr_v->row[2] = rcp_area * (a0.z * w0_row + a1.z * w1_row + a2.z * w2_row);
            } break;
            
            case FRAG_ATTR_U32:   {
                curr_v->component_count = 3;
                curr_v->perspective_correct = attr0->perspective_correct;

                u32 color0_u32 = *(u32*)attr0->data;
                u32 color1_u32 = *(u32*)attr1->data;
                u32 color2_u32 = *(u32*)attr2->data;

                float r0 = (float)((color0_u32 >> 16) & 0xFF);
                float g0 = (float)((color0_u32 >>  8) & 0xFF);
                float b0 = (float)( color0_u32        & 0xFF);

                float r1 = (float)((color1_u32 >> 16) & 0xFF);
                float g1 = (float)((color1_u32 >>  8) & 0xFF);
                float b1 = (float)( color1_u32        & 0xFF);

                float r2 = (float)((color2_u32 >> 16) & 0xFF);
                float g2 = (float)((color2_u32 >>  8) & 0xFF);
                float b2 = (float)( color2_u32        & 0xFF);

                if (attr0->perspective_correct) {
                    r0 *= rcp_w0; g0 *= rcp_w0; b0 *= rcp_w0;
                    r1 *= rcp_w1; g1 *= rcp_w1; b1 *= rcp_w1;
                    r2 *= rcp_w2; g2 *= rcp_w2; b2 *= rcp_w2;
                }

                curr_v->dx[0] = rcp_area * (r0 * dx0 + r1 * dx1 + r2 * dx2);
                curr_v->dy[0] = rcp_area * (r0 * dy0 + r1 * dy1 + r2 * dy2);
                curr_v->row[0] = rcp_area * (r0 * w0_row + r1 * w1_row + r2 * w2_row);

                curr_v->dx[1] = rcp_area * (g0 * dx0 + g1 * dx1 + g2 * dx2);
                curr_v->dy[1] = rcp_area * (g0 * dy0 + g1 * dy1 + g2 * dy2);
                curr_v->row[1] = rcp_area * (g0 * w0_row + g1 * w1_row + g2 * w2_row);

                curr_v->dx[2] = rcp_area * (b0 * dx0 + b1 * dx1 + b2 * dx2);
                curr_v->dy[2] = rcp_area * (b0 * dy0 + b1 * dy1 + b2 * dy2);
                curr_v->row[2] = rcp_area * (b0 * w0_row + b1 * w1_row + b2 * w2_row);
            } break;
            
            default: break;
        }
    }

    intptr_t row_offset = clamped_min_y * win_width;
    for (int y = clamped_min_y; y <= clamped_max_y; ++y) {
        float* restrict z_ptr = ctx->framebuffer.depth_buffer + row_offset + clamped_min_x;
        u32* restrict color_ptr = ctx->framebuffer.color_buffer + row_offset + clamped_min_x;

        float w0_start = w0_row;
        float w1_start = w1_row;
        float w2_start = w2_row;
        float depth    = depth_recip_row;

        for (u32 attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
            interpolator_t *curr_v = &interpolators[attr_idx];
            for (int c = 0; c < curr_v->component_count; c++) {
                curr_v->v[c] = curr_v->row[c];
            }
        }
        
        for (int x = clamped_min_x; x <= clamped_max_x; x++) {
            if (w0_start >= 0 && w1_start >= 0 && w2_start >= 0 && depth > *z_ptr) {
                const float depth_recip = 1.0f/depth;
                u32 final_color;

                if (ctx->fragment_shader) {
                    final_color = ctx->fragment_shader((const render_context*)ctx, (const interpolator_t*)interpolators, depth_recip);
                } else {
                    final_color = 0xFFFF00FF;
                }

                *z_ptr = depth;
                *color_ptr = final_color;
            }
            
            
            w0_start += dx0;
            w1_start += dx1;
            w2_start += dx2;
            depth    += depth_dx;
            z_ptr++;
            color_ptr++;
            for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
                interpolator_t *restrict curr_v = &interpolators[attr_idx];
                switch (curr_v->component_count) {
                    case 4: curr_v->v[3] += curr_v->dx[3];
                    case 3: curr_v->v[2] += curr_v->dx[2];
                    case 2: curr_v->v[1] += curr_v->dx[1];
                    case 1: curr_v->v[0] += curr_v->dx[0];
                }
            }
        }

        for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
            interpolator_t *restrict curr_v = &interpolators[attr_idx];
            switch (curr_v->component_count) {
                case 4: curr_v->row[3] += curr_v->dy[3];
                case 3: curr_v->row[2] += curr_v->dy[2];
                case 2: curr_v->row[1] += curr_v->dy[1];
                case 1: curr_v->row[0] += curr_v->dy[0];
            }
        }

        w0_row += dy0;
        w1_row += dy1;
        w2_row += dy2;
        depth_recip_row += depth_dy;
        row_offset += win_width;
    }
}

void draw_triangle_modular_BACKUP(render_context *ctx, 
    float x0, float y0, float w0,
    float x1, float y1, float w1,
    float x2, float y2, float w2,
    vertex_attributes_t *v0, vertex_attributes_t *v1, vertex_attributes_t *v2) {
    
    float area = ((x2 - x0) * (y1 - y0)) - ((y2 - y0) * (x1 - x0));
    if (area < 0) {
        area = -area;
        swap(x1,x2);
        swap(y1,y2);
        swap(w1,w2);
        swap(v1,v2);
    }

    if (fabsf(area) < 1e-6f) return;

    const int win_width = ctx->framebuffer.width;
    const int win_height = ctx->framebuffer.height;

    const int min_x_f = (int) floorf(fmin(fmin(x0, x1), x2));
    const int min_y_f = (int) floorf(fmin(fmin(y0, y1), y2));
    const int max_x_f = (int)  ceilf(fmax(fmax(x0, x1), x2));
    const int max_y_f = (int)  ceilf(fmax(fmax(y0, y1), y2));

    const int clamped_min_x = (min_x_f < 0) ? 0 : min_x_f;
    const int clamped_min_y = (min_y_f < 0) ? 0 : min_y_f;
    const int clamped_max_x = (max_x_f >= win_width) ? (win_width - 1) : max_x_f;
    const int clamped_max_y = (max_y_f >= win_height) ? (win_height - 1) : max_y_f;

    if (clamped_min_x > clamped_max_x || clamped_min_y > clamped_max_y) return;

    const float rcp_area = 1.0f / area;
    const float rcp_w0 = 1.0f / w0;
    const float rcp_w1 = 1.0f / w1;
    const float rcp_w2 = 1.0f / w2;
    
    const float dx0 = y2 - y1;
    const float dy0 = x1 - x2;
    const float dx1 = y0 - y2;
    const float dy1 = x2 - x0;
    const float dx2 = y1 - y0;
    const float dy2 = x0 - x1;

    const float depth_dx = rcp_area * (rcp_w0 * dx0 + rcp_w1 * dx1 + rcp_w2 * dx2);
    const float depth_dy = rcp_area * (rcp_w0 * dy0 + rcp_w1 * dy1 + rcp_w2 * dy2);

    const float psx = clamped_min_x+0.5f;
    const float psy = clamped_min_y+0.5f;
    float w0_row = (psx - x1) * (y2 - y1) - (psy - y1) * (x2 - x1);
    float w1_row = (psx - x2) * (y0 - y2) - (psy - y2) * (x0 - x2);
    float w2_row = (psx - x0) * (y1 - y0) - (psy - y0) * (x1 - x0);
    float depth_recip_row = rcp_area * (rcp_w0 * w0_row + rcp_w1 * w1_row + rcp_w2 * w2_row);

    interpolator_t interpolators[v0->attribute_count]; // surly the user will pass the same number of attributes for each vertex
    for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
        interpolators[attr_idx] = (interpolator_t){0};
        interpolator_t *curr_v = &interpolators[attr_idx];
        
        // attribute for each vertex
        fragment_attribute_t *attr0 = &v0->attributes[attr_idx];
        fragment_attribute_t *attr1 = &v1->attributes[attr_idx];
        fragment_attribute_t *attr2 = &v2->attributes[attr_idx];

        switch (attr0->type) {
            case FRAG_ATTR_FLOAT: {
                curr_v->component_count = 1;
                curr_v->perspective_correct = attr0->perspective_correct;

                float a0 = *(float*)attr0->data;
                float a1 = *(float*)attr1->data;
                float a2 = *(float*)attr2->data; 
                
                if (attr0->perspective_correct) {
                    a0 *= rcp_w0;
                    a1 *= rcp_w1;
                    a2 *= rcp_w2;
                } 

                curr_v->dx[0]  = rcp_area * (a0 * dx0    + a1 * dx1    + a2 * dx2   );
                curr_v->dy[0]  = rcp_area * (a0 * dy0    + a1 * dy1    + a2 * dy2   );
                curr_v->row[0] = rcp_area * (a0 * w0_row + a1 * w1_row + a2 * w2_row); 
            } break;
            
            case FRAG_ATTR_VEC2:  {
                curr_v->component_count = 2;
                curr_v->perspective_correct = attr0->perspective_correct;

                vec2 a0 = *(vec2*)attr0->data;
                vec2 a1 = *(vec2*)attr1->data;
                vec2 a2 = *(vec2*)attr2->data;

                if (attr0->perspective_correct) {
                    a0.x *= rcp_w0; a0.y *= rcp_w0;
                    a1.x *= rcp_w1; a1.y *= rcp_w1;
                    a2.x *= rcp_w2; a2.y *= rcp_w2;
                }

                curr_v->dx[0]  = rcp_area * (a0.x * dx0    + a1.x * dx1    + a2.x * dx2   );
                curr_v->dy[0]  = rcp_area * (a0.x * dy0    + a1.x * dy1    + a2.x * dy2   );
                curr_v->row[0] = rcp_area * (a0.x * w0_row + a1.x * w1_row + a2.x * w2_row);

                curr_v->dx[1]  = rcp_area * (a0.y * dx0    + a1.y * dx1    + a2.y * dx2   );
                curr_v->dy[1]  = rcp_area * (a0.y * dy0    + a1.y * dy1    + a2.y * dy2   );
                curr_v->row[1] = rcp_area * (a0.y * w0_row + a1.y * w1_row + a2.y * w2_row);
            } break;
            
            case FRAG_ATTR_VEC3:  {
                curr_v->component_count = 3;
                curr_v->perspective_correct = attr0->perspective_correct;

                vec3 a0 = *(vec3*)attr0->data;
                vec3 a1 = *(vec3*)attr1->data;
                vec3 a2 = *(vec3*)attr2->data;

                if (attr0->perspective_correct) {
                    a0.x *= rcp_w0; a0.y *= rcp_w0; a0.z *= rcp_w0;
                    a1.x *= rcp_w1; a1.y *= rcp_w1; a1.z *= rcp_w1;
                    a2.x *= rcp_w2; a2.y *= rcp_w2; a2.z *= rcp_w2;
                }

                curr_v->dx[0]  = rcp_area * (a0.x * dx0    + a1.x * dx1    + a2.x * dx2   );
                curr_v->dy[0]  = rcp_area * (a0.x * dy0    + a1.x * dy1    + a2.x * dy2   );
                curr_v->row[0] = rcp_area * (a0.x * w0_row + a1.x * w1_row + a2.x * w2_row);

                curr_v->dx[1]  = rcp_area * (a0.y * dx0    + a1.y * dx1    + a2.y * dx2   );
                curr_v->dy[1]  = rcp_area * (a0.y * dy0    + a1.y * dy1    + a2.y * dy2   );
                curr_v->row[1] = rcp_area * (a0.y * w0_row + a1.y * w1_row + a2.y * w2_row);

                curr_v->dx[2]  = rcp_area * (a0.z * dx0    + a1.z * dx1    + a2.z * dx2   );
                curr_v->dy[2]  = rcp_area * (a0.z * dy0    + a1.z * dy1    + a2.z * dy2   );
                curr_v->row[2] = rcp_area * (a0.z * w0_row + a1.z * w1_row + a2.z * w2_row);
            } break;
            
            case FRAG_ATTR_U32:   {
                curr_v->component_count = 3;
                curr_v->perspective_correct = attr0->perspective_correct;

                u32 color0_u32 = *(u32*)attr0->data;
                u32 color1_u32 = *(u32*)attr1->data;
                u32 color2_u32 = *(u32*)attr2->data;

                float r0 = (float)((color0_u32 >> 16) & 0xFF);
                float g0 = (float)((color0_u32 >>  8) & 0xFF);
                float b0 = (float)( color0_u32        & 0xFF);

                float r1 = (float)((color1_u32 >> 16) & 0xFF);
                float g1 = (float)((color1_u32 >>  8) & 0xFF);
                float b1 = (float)( color1_u32        & 0xFF);

                float r2 = (float)((color2_u32 >> 16) & 0xFF);
                float g2 = (float)((color2_u32 >>  8) & 0xFF);
                float b2 = (float)( color2_u32        & 0xFF);

                if (attr0->perspective_correct) {
                    r0 *= rcp_w0; g0 *= rcp_w0; b0 *= rcp_w0;
                    r1 *= rcp_w1; g1 *= rcp_w1; b1 *= rcp_w1;
                    r2 *= rcp_w2; g2 *= rcp_w2; b2 *= rcp_w2;
                }

                curr_v->dx[0] = rcp_area * (r0 * dx0 + r1 * dx1 + r2 * dx2);
                curr_v->dy[0] = rcp_area * (r0 * dy0 + r1 * dy1 + r2 * dy2);
                curr_v->row[0] = rcp_area * (r0 * w0_row + r1 * w1_row + r2 * w2_row);

                curr_v->dx[1] = rcp_area * (g0 * dx0 + g1 * dx1 + g2 * dx2);
                curr_v->dy[1] = rcp_area * (g0 * dy0 + g1 * dy1 + g2 * dy2);
                curr_v->row[1] = rcp_area * (g0 * w0_row + g1 * w1_row + g2 * w2_row);

                curr_v->dx[2] = rcp_area * (b0 * dx0 + b1 * dx1 + b2 * dx2);
                curr_v->dy[2] = rcp_area * (b0 * dy0 + b1 * dy1 + b2 * dy2);
                curr_v->row[2] = rcp_area * (b0 * w0_row + b1 * w1_row + b2 * w2_row);
            } break;
            
            default: break;
        }
    }

    intptr_t row_offset = clamped_min_y * win_width;
    for (int y = clamped_min_y; y <= clamped_max_y; ++y) {
        float* z_ptr = ctx->framebuffer.depth_buffer + row_offset + clamped_min_x;
        u32* color_ptr = ctx->framebuffer.color_buffer + row_offset + clamped_min_x;

        float w0_start = w0_row;
        float w1_start = w1_row;
        float w2_start = w2_row;
        float depth    = depth_recip_row;

        // reset interpolated attributes for the start of the row here
        for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
            interpolator_t *curr_v = &interpolators[attr_idx];
            
            for (int c = 0; c < curr_v->component_count; c++) {
                curr_v->v[c] = curr_v->row[c];
            }
        }
        
        for (int x = clamped_min_x; x <= clamped_max_x; x++) {
            if (w0_start >= 0 && w1_start >= 0 && w2_start >= 0 && depth > *z_ptr) {
                const float inv_w = 1.0f / depth;
                u32 final_color = 0;

                // TODO: attribute interpolation
                for (int attr_idx = 0; attr_idx < v0->attribute_count; ++attr_idx) {
                    fragment_attribute_t* attr = &v0->attributes[attr_idx];
                    interpolator_t* interp = &interpolators[attr_idx];

                    if (attr->type == FRAG_ATTR_U32) {
                        float r = interp->v[0];
                        float g = interp->v[1];
                        float b = interp->v[2];

                        if (interp->perspective_correct) {
                            r *= inv_w;
                            g *= inv_w;
                            b *= inv_w;
                        }

                        // Clamp and pack
                        u32 r_u32 = (u32)((r < 0) ? 0 : (r > 255) ? 255 : r);
                        u32 g_u32 = (u32)((g < 0) ? 0 : (g > 255) ? 255 : g);
                        u32 b_u32 = (u32)((b < 0) ? 0 : (b > 255) ? 255 : b);

                        final_color = 0xFF000000 | (r_u32 << 16) | (g_u32 << 8) | b_u32;
                    }
                    // else if (attr->type == FRAG_ATTR_VEC2) { ... use for texture mapping ... }
                }

                // fragment shader will go here, user defines a function 
                // void fragment_shader(vertex_attributes_t *v0, vertex_attributes_t *v1, vertex_attributes_t *v2, vec3 position, u32 *out_color);
                
                // for now we manually do things here

                *z_ptr = depth;
                *color_ptr = final_color;
            }
            
            
            w0_start += dx0;
            w1_start += dx1;
            w2_start += dx2;
            depth    += depth_dx;
            z_ptr++;
            color_ptr++;
            for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
                interpolator_t *curr_v = &interpolators[attr_idx];
                switch (curr_v->component_count) {
                    case 4: curr_v->v[3] += curr_v->dx[3];
                    case 3: curr_v->v[2] += curr_v->dx[2];
                    case 2: curr_v->v[1] += curr_v->dx[1];
                    case 1: curr_v->v[0] += curr_v->dx[0];
                }
            }
        }

        // increment interpolated attributes for row here
        for (int attr_idx = 0; attr_idx < v0->attribute_count; attr_idx++) {
            interpolator_t *curr_v = &interpolators[attr_idx];
            switch (curr_v->component_count) {
                case 4: curr_v->row[3] += curr_v->dy[3];
                case 3: curr_v->row[2] += curr_v->dy[2];
                case 2: curr_v->row[1] += curr_v->dy[1];
                case 1: curr_v->row[0] += curr_v->dy[0];
            }
        }

        w0_row += dy0;
        w1_row += dy1;
        w2_row += dy2;
        depth_recip_row += depth_dy;
        row_offset += win_width;
    }
}


/*

draw modes

FLAT
GOURAUD
TEXTURED
TEXTURED_GOURAUD
PHONG..?


*/