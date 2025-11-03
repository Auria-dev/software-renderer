#include "graphics.h"

// scalar, gouraud, textured
#ifndef draw_triangle_sgt
#define RASTERIZER_NAME draw_triangle_sgt
#define RASTER_GOURAUD  1
#define RASTER_TEXTURE  1
#include "triangle_template.h"
#endif

// scalar, gouraud, colored
#ifndef draw_triangle_sgc
#define RASTERIZER_NAME draw_triangle_sgc
#define RASTER_GOURAUD  1
#define RASTER_TEXTURE  0
#include "triangle_template.h"
#endif

// scalar, flat, textured
#ifndef draw_triangle_sft
#define RASTERIZER_NAME draw_triangle_sft
#define RASTER_GOURAUD  0
#define RASTER_TEXTURE  1
#include "triangle_template.h"
#endif

// scalar, flat, colored
#ifndef draw_triangle_sfc
#define RASTERIZER_NAME draw_triangle_sfc
#define RASTER_GOURAUD  0
#define RASTER_TEXTURE  0
#include "triangle_template.h"
#endif

// scalar, gouraud, textured, bilinear sampling
#ifndef draw_triangle_sgtb
#define RASTERIZER_NAME draw_triangle_sgtb
#define RASTER_GOURAUD  1
#define RASTER_TEXTURE  1
#define SAMPLE_BILINEAR
#include "triangle_template.h"
#endif

// scalar, gouraud, colored, bilinear sampling
#ifndef draw_triangle_sgcb
#define RASTERIZER_NAME draw_triangle_sgcb
#define RASTER_GOURAUD  1
#define RASTER_TEXTURE  0
#define SAMPLE_BILINEAR
#include "triangle_template.h"
#endif

// scalar, flat, textured, bilinear sampling
#ifndef draw_triangle_sftb
#define RASTERIZER_NAME draw_triangle_sftb
#define RASTER_GOURAUD  0
#define RASTER_TEXTURE  1
#define SAMPLE_BILINEAR
#include "triangle_template.h"
#endif

// scalar, flat, colored, bilinear sampling
#ifndef draw_triangle_sfcb
#define RASTERIZER_NAME draw_triangle_sfcb
#define RASTER_GOURAUD  0
#define RASTER_TEXTURE  0
#define SAMPLE_BILINEAR
#include "triangle_template.h"
#endif

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
    ctx.clip_near = near;
    ctx.clip_far = far;
    ctx.depth_test = enable_depth_test;
    ctx.blend_test = enable_blend_test;
    ctx.cull_face = enable_cull_face;
    ctx.material_id = -1;
    ctx.frustum = frustum_init(fov, aspect_ratio, near, far);

    ctx.material_manager = malloc(sizeof(material_manager_t));
    *ctx.material_manager = m_init();
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

// helper functions
static u32 pack_color(vec3 color) {
    // Clamp color to valid 0.0-1.0 range
    color.x = fmax(0.0f, fmin(1.0f, color.x));
    color.y = fmax(0.0f, fmin(1.0f, color.y));
    color.z = fmax(0.0f, fmin(1.0f, color.z));

    u32 r = (u32)(color.x * 255.0f);
    u32 g = (u32)(color.y * 255.0f);
    u32 b = (u32)(color.z * 255.0f);
    
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
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

void g_set_bilinear_sampling(render_context *ctx, bool enabled) {
    ctx->bilinear_sampling = enabled;
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

void g_draw_mesh(render_context* ctx, mesh_t* mesh, int type, int render_mode) {
    g_update_world_matrix(ctx, mesh->position, mesh->rotation, mesh->scale);

    for (int i = 0; i < mesh->submesh_count; i++) {
        submesh_t* sub = &mesh->submeshes[i];

        material_t* mat = m_get_material(ctx->material_manager, sub->material_id);
        ctx->current_material = mat; // set for g_draw_elements
        bool has_texture = (mat && mat->diffuse_map_id != -1);

        // TODO: flag system to not have to deal with SGT, SGC, SFT, SFC manually
        if (has_texture) {
            ctx->current_texture = m_get_texture(ctx->material_manager, mat->diffuse_map_id);

            if (type == MESH_GOURAUD) 
                ctx->current_shader = SHADER_SGT;
            else
                ctx->current_shader = SHADER_SFT;
            
        } else {
            ctx->current_texture = NULL;
            
            if (type == MESH_GOURAUD) 
                ctx->current_shader = SHADER_SGC;
            else 
                ctx->current_shader = SHADER_SFC;
        }

        g_bind_material(ctx, sub->material_id);
        g_bind_buffer(ctx, GBUFFER_VERTEX, mesh->vertices, mesh->vertex_count * sizeof(vertex_t));
        g_bind_buffer(ctx, GBUFFER_INDEX, sub->indices, sub->index_count * sizeof(u32));

        g_draw_elements(ctx, sub->index_count, sub->indices, render_mode);
    }

    // unbind
    ctx->current_material = NULL;
    ctx->current_texture = NULL;
}

void g_draw_elements(render_context *ctx, u32 count, u32 *indices, int render_mode) {
    material_t* mat = ctx->current_material;
    
    material_t default_mat = {
        .ambient = {0.1f, 0.1f, 0.1f},
        .diffuse = {1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f},
        .shininess = 32.0f,
        .diffuse_map_id = -1,
        .color = 0xFFFFFFFF
    };

    if (!mat) {
        mat = &default_mat;
        // printf("DEBUG: g_draw_elements: using default material\n");
    } 
    
    vec3 light_dir_view = vec3_normalize((vec3){0.5f, 0.5f, -1.0f});
    vec3 ambient_light_color = (vec3){0.2f, 0.2f, 0.2f};
    vec3 diffuse_light_color = (vec3){0.8f, 0.8f, 0.8f};
    
    mat4 transform = mat4_mul_mat4(ctx->view_matrix, ctx->world_matrix);
    mat3 normal_matrix = mat4_to_mat3(mat4_transpose(mat4_inverse(transform)));

    for (u32 i = 0; i < count; i += 3) {
        u32 vi0 = indices[i+0];
        u32 vi1 = indices[i+1];
        u32 vi2 = indices[i+2];

        vertex_t v0 = ((vertex_t*)ctx->vertex_buffer.data)[vi0];
        vertex_t v1 = ((vertex_t*)ctx->vertex_buffer.data)[vi1];
        vertex_t v2 = ((vertex_t*)ctx->vertex_buffer.data)[vi2];

        // apply world and view transformations
        v0.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v0.position)));
        v1.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v1.position)));
        v2.position = vec4_to_vec3(mat4_mul_vec4(transform, vec3_to_vec4(v2.position)));

        vec3 a = vec3_sub(v1.position, v0.position);
        vec3 b = vec3_sub(v2.position, v0.position);
        vec3 face_normal = vec3_normalize(vec3_cross(a, b));

        if (ctx->cull_face) {
            vec3 view_dir = vec3_normalize(v0.position);
            if (vec3_dot(face_normal, view_dir) > 0) {
                continue;
            }
        }

        v0.color = 0xff00ff00;
        v1.color = 0xff00ff00;
        v2.color = 0xff00ff00;

        switch (ctx->current_shader) {
            
            case SHADER_SGC: {
                vec3 n0 = vec3_normalize(mat3_mul_vec3(normal_matrix, v0.normal));
                vec3 n1 = vec3_normalize(mat3_mul_vec3(normal_matrix, v1.normal));
                vec3 n2 = vec3_normalize(mat3_mul_vec3(normal_matrix, v2.normal));

                vec3 amb = vec3_mul(mat->ambient, ambient_light_color);
                
                vec3 c0_f = vec3_add(amb, vec3_scale(vec3_mul(mat->diffuse, diffuse_light_color), fmax(0.0, vec3_dot(n0, light_dir_view))));
                vec3 c1_f = vec3_add(amb, vec3_scale(vec3_mul(mat->diffuse, diffuse_light_color), fmax(0.0, vec3_dot(n1, light_dir_view))));
                vec3 c2_f = vec3_add(amb, vec3_scale(vec3_mul(mat->diffuse, diffuse_light_color), fmax(0.0, vec3_dot(n2, light_dir_view))));
                
                v0.color = pack_color(c0_f);
                v1.color = pack_color(c1_f);
                v2.color = pack_color(c2_f);
                break;
            }

            case SHADER_SFC: {
                vec3 amb = vec3_mul(mat->ambient, ambient_light_color);
                vec3 c_f = vec3_add(amb, vec3_scale(vec3_mul(mat->diffuse, diffuse_light_color), fmax(0.0, vec3_dot(face_normal, light_dir_view))));
                
                u32 face_color = pack_color(c_f);
                v0.color = face_color;
                v1.color = face_color;
                v2.color = face_color;
                break;
            }

            case SHADER_SGT: {
                vec3 n0 = vec3_normalize(mat3_mul_vec3(normal_matrix, v0.normal));
                vec3 n1 = vec3_normalize(mat3_mul_vec3(normal_matrix, v1.normal));
                vec3 n2 = vec3_normalize(mat3_mul_vec3(normal_matrix, v2.normal));

                vec3 c0_f = vec3_add(ambient_light_color, vec3_scale(diffuse_light_color, fmax(0.0, vec3_dot(n0, light_dir_view))));
                vec3 c1_f = vec3_add(ambient_light_color, vec3_scale(diffuse_light_color, fmax(0.0, vec3_dot(n1, light_dir_view))));
                vec3 c2_f = vec3_add(ambient_light_color, vec3_scale(diffuse_light_color, fmax(0.0, vec3_dot(n2, light_dir_view))));

                v0.color = pack_color(c0_f);
                v1.color = pack_color(c1_f);
                v2.color = pack_color(c2_f);
                break;
            }

            case SHADER_SFT: {
                vec3 c_f = vec3_add(ambient_light_color, vec3_scale(diffuse_light_color, fmax(0.0, vec3_dot(face_normal, light_dir_view))));
                
                u32 face_color = pack_color(c_f);
                v0.color = face_color;
                v1.color = face_color;
                v2.color = face_color;
                break;
            }
        }
        
        v0.color = 0xffffffff;
        v1.color = 0xffffffff;
        v2.color = 0xffffffff;

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

            u32 material_color = mat->color;

            switch (render_mode) {
                case 0: {
                    // textured drawing
                    draw_triangle(
                        ctx,
                        ctx->current_shader,
                        screen0_x, screen0_y, pv0.w, tv0.texcoord.x, tv0.texcoord.y, tv0.color,
                        screen1_x, screen1_y, pv1.w, tv1.texcoord.x, tv1.texcoord.y, tv1.color,
                        screen2_x, screen2_y, pv2.w, tv2.texcoord.x, tv2.texcoord.y, tv2.color
                    );
                } break;
                case 1: {
                    // material color drawing
                    draw_triangle(
                        ctx,
                        SHADER_SFC,
                        screen0_x, screen0_y, pv0.w, tv0.texcoord.x, tv0.texcoord.y, material_color,
                        screen1_x, screen1_y, pv1.w, tv1.texcoord.x, tv1.texcoord.y, material_color,
                        screen2_x, screen2_y, pv2.w, tv2.texcoord.x, tv2.texcoord.y, material_color
                    );
                } break;
                case 2: {
                    // wireframe drawing
                    draw_line(ctx, (int)screen0_x, (int)screen0_y, (int)screen1_x, (int)screen1_y, material_color);
                    draw_line(ctx, (int)screen1_x, (int)screen1_y, (int)screen2_x, (int)screen2_y, material_color);
                    draw_line(ctx, (int)screen2_x, (int)screen2_y, (int)screen0_x, (int)screen0_y, material_color);
                } break;
                case 3: {
                    // normal drawing
                    vec3 normal_color0 = vec3_scale(vec3_add(tv0.normal, (vec3){1.0f,1.0f,1.0f}), 0.5f);
                    vec3 normal_color1 = vec3_scale(vec3_add(tv1.normal, (vec3){1.0f,1.0f,1.0f}), 0.5f);
                    vec3 normal_color2 = vec3_scale(vec3_add(tv2.normal, (vec3){1.0f,1.0f,1.0f}), 0.5f);
                    draw_triangle(
                        ctx,
                        SHADER_SGC,
                        screen0_x, screen0_y, pv0.w, tv0.texcoord.x, tv0.texcoord.y, pack_color(normal_color0),
                        screen1_x, screen1_y, pv1.w, tv1.texcoord.x, tv1.texcoord.y, pack_color(normal_color1),
                        screen2_x, screen2_y, pv2.w, tv2.texcoord.x, tv2.texcoord.y, pack_color(normal_color2)
                    );
                } break;
            }
        }
    }
}

void draw_triangle(
    render_context* ctx,
    shader_type_t shader_type,
    float x0, float y0, float w0, float u0, float v0, u32 c0,
    float x1, float y1, float w1, float u1, float v1, u32 c1,
    float x2, float y2, float w2, float u2, float v2, u32 c2) {
    
    if (ctx->bilinear_sampling) {
        switch (shader_type) {
            case SHADER_SGT: {
                draw_triangle_sgtb(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SGC: {
                draw_triangle_sgcb(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SFT: {
                draw_triangle_sftb(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SFC: {
                draw_triangle_sfcb(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
        }
    } else {
        switch (shader_type) {
            case SHADER_SGT: {
                draw_triangle_sgt(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SGC: {
                draw_triangle_sgc(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SFT: {
                draw_triangle_sft(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
            case SHADER_SFC: {
                draw_triangle_sfc(ctx, x0, y0, w0, u0, v0, c0, x1, y1, w1, u1, v1, c1, x2, y2, w2, u2, v2, c2);
            } break;
        }
    }
}
