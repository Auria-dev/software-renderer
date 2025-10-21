#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "c3m.h"
#include "vertex.h"
#include "textures.h"
#include "clipping.h"

// forward declarations
struct render_context;
struct interpolator_t;

enum {
  GBUFFER_INDEX,
  GBUFFER_VERTEX
};

typedef struct {
  void*  data;
  size_t size;
  u8    type;
} buffer_t;

enum {
    FRAG_ATTR_FLOAT,
    FRAG_ATTR_VEC2,
    FRAG_ATTR_VEC3,
    FRAG_ATTR_VEC4,
    FRAG_ATTR_U32
};

typedef struct {
	float v[4];
	float dx[4];
	float dy[4];
	float row[4];
	u8 component_count;
	bool perspective_correct;
} interpolator_t;

typedef struct {
    void* data;
    u8 type;
	bool perspective_correct;
} fragment_attribute_t;

// this is an array/list of all the attributes that a triangle will have that will get interpolated.
// for now we will always interpolate depth (1/w) even if depth_test is disabled, because it's needed for perspective correct interpolation.
typedef struct {
	fragment_attribute_t* attributes;
    u32 attribute_count;
} vertex_attributes_t; // a better name for this might be "vertex_attributes" 

enum {
    FRUSTUM_LEFT,
    FRUSTUM_RIGHT,
    FRUSTUM_TOP,
    FRUSTUM_BOTTOM,
    FRUSTUM_NEAR,
    FRUSTUM_FAR
};

typedef struct {
    clipping_plane_t planes[6];
} frustum_t;

typedef struct {
  u32* color_buffer;
  float* depth_buffer;
  int width, height;
} framebuffer_t;

typedef u32 (*fragment_shader_fn)(
    const struct render_context *ctx, 
    const interpolator_t *interpolators, 
    float inv_w
);

typedef struct render_context {
  mat4 projection_matrix;
  mat4 world_matrix;
  mat4 view_matrix;

  frustum_t frustum;
  framebuffer_t framebuffer;

  buffer_t vertex_buffer;
  buffer_t index_buffer;
  int material_id;
  
  texture_manager_t texture_manager;
  fragment_shader_fn fragment_shader;

  float clip_near;
  float clip_far;

  bool depth_test;
  bool blend_test;
  bool cull_face;
} render_context;

void draw_pixel(render_context *ctx, int x, int y, u32 c);

framebuffer_t framebuffer_init(int width, int height);
frustum_t frustum_init(float fov, float aspect_ratio, float clipping_near, float clipping_far);

render_context render_context_init(
    int width, int height,
    float fov, float aspect_ratio, float near, float far,
    vec3 cam_pos, vec3 cam_target, vec3 cam_up,
    bool enable_depth_test, bool enable_blend_test, bool enable_cull_face);

void g_update_projection_matrix(render_context *ctx, float fov, float ar);
void g_update_view_matrix(render_context *ctx, mat4 view);
void g_update_world_matrix(render_context *ctx, vec3 position, vec3 rotation, vec3 scale);

void g_bind_material(render_context *ctx, int material_id);
void g_bind_buffer(render_context *ctx, u32 type, void* data, size_t size);
void g_bind_fragment_shader(render_context *ctx, fragment_shader_fn shader);

void g_draw_elements(render_context *ctx, u32 count, u32 *indices);

// temporary until modular attributes/shaders are implemented
void draw_triangle_scalar(
        render_context *ctx,
        float x0, float y0, float w0, float u0, float v0, u32 c0,
        float x1, float y1, float w1, float u1, float v1, u32 c1,
        float x2, float y2, float w2, float u2, float v2, u32 c2);

void draw_triangle_modular(render_context *ctx, 
    float x0, float y0, float w0,
    float x1, float y1, float w1,
    float x2, float y2, float w2,
    vertex_attributes_t *v0, vertex_attributes_t *v1, vertex_attributes_t *v2);

#endif // GRAPHICS_H