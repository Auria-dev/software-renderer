#ifndef C3M_H
#define C3M_H

// C 3D Math library
// Uses column-major matrices
// Vectors are treated as column vectors

#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>

#ifndef EPS
    #define EPS 1e-6f
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846f
#endif

typedef uint32_t u32;

typedef struct { float x, y; } vec2;
typedef struct { float x, y, z; } vec3;
typedef struct { float x, y, z, w; } vec4;
typedef vec4 quat;

typedef struct {
    float m[3][3];
} mat3;

typedef struct {
    float m[4][4];
} mat4;

vec2  vec2_new(float x, float y);
vec2  vec2_add(vec2 a, vec2 b);
vec2  vec2_sub(vec2 a, vec2 b);
vec2  vec2_mul(vec2 a, vec2 b);
vec2  vec2_div(vec2 a, vec2 b);
vec2  vec2_scale(vec2 a, float s);
vec2  vec2_normalize(vec2 v);
bool  vec2_eq(vec2 a, vec2 b);
vec2  vec2_clone(vec2 *v);
vec2  vec2_lerp(vec2 a, vec2 b, float t);
float vec2_dot(vec2 a, vec2 b);
float vec2_len(vec2 v);
float vec2_len_sq(vec2 v);

bool  vec3_eq(vec3 a, vec3 b);
vec3  vec3_new(float x, float y, float z);
vec3  vec3_add(vec3 a, vec3 b);
vec3  vec3_sub(vec3 a, vec3 b);
vec3  vec3_scale(vec3 v, float s);
vec3  vec3_div(vec3 v, float s);
float vec3_dot(vec3 a, vec3 b);
vec3  vec3_cross(vec3 a, vec3 b);
float vec3_len_sq(vec3 v);
float vec3_len(vec3 v);
vec3  vec3_normalize(vec3 v);
vec3  vec3_lerp(vec3 a, vec3 b, float t);
vec3  vec3_reflect(vec3 v, vec3 n);
vec3  vec3_negate(vec3 v);
vec3  vec3_zero();
vec3  vec3_left();
vec3  vec3_up();
vec3  vec3_forward();
vec3  vec3_clone(vec3* v);
vec3  vec3_min(vec3 a, vec3 b);
vec3  vec3_max(vec3 a, vec3 b);
vec3  vec3_orthogonal(vec3 v);

vec4  vec4_new(float x, float y, float z, float w);
vec4  vec4_add(vec4 a, vec4 b);
vec4  vec4_sub(vec4 a, vec4 b);
vec4  vec4_scale(vec4 v, float s);
float vec4_dot(vec4 a, vec4 b);
float vec4_len_sq(vec4 v);
float vec4_len(vec4 v);
vec4  vec4_normalize(vec4 v);
vec4  vec4_lerp(vec4 a, vec4 b, float t);

quat  quat_new(float x, float y, float z, float w);
quat  quat_identity();
quat  quat_add(quat a, quat b);
quat  quat_sub(quat a, quat b);
quat  quat_scale(quat v, float s);
quat  quat_mul(quat p, quat q);
quat  quat_conjugate(quat q);
quat  quat_inverse(quat q);
vec3  quat_rotate_vec3(quat q, vec3 v);
quat  quat_from_axis_angle(vec3 axis, float angle_rad);
quat  quat_slerp(quat a, quat b, float t);
quat  quat_from_to(vec3 u, vec3 v); // finds shortest rotation U should take to point in the same direction as V
float quat_length_sq(quat q);
float quat_length(quat q);
quat  quat_normalize(quat q);
quat  quat_from_vec3_angle(vec3 v, float half_angle);
mat3  quat_to_mat3(quat q);

mat3  mat3_identity(void);
mat3  mat3_zero(void);
mat3  mat3_init(float a, float b, float c, float d, float e, float f, float g, float h, float i);
vec3  mat3_mul_vec3(mat3 m, vec3 v);
mat3  mat3_mul_mat3(mat3 a, mat3 b);
mat3  mat3_transpose(mat3 m);
mat3  mat3_scale(mat3 m, float s);
mat3  mat3_inverse_diagonal(mat3 m);
mat3  mat3_inverse(mat3 m);

mat4  mat4_identity(void);
mat4  mat4_from_pos_quat(vec3 v, quat q);
mat4  mat4_make_scale(float sx, float sy, float sz);
mat4  mat4_make_translation(float tx, float ty, float tz);
mat4  mat4_make_perspective(float fov, float aspect, float znear, float zfar);
vec4  mat4_mul_vec4(mat4 m, vec4 v);
vec4  mat4_mul_vec4_project(mat4 mat_proj, vec4 v);
mat4  mat4_make_rotation_x(float a);
mat4  mat4_make_rotation_y(float a);
mat4  mat4_make_rotation_z(float a);
vec3  mat4_mul_vec3(mat4 m, vec3 v);
mat4  mat4_mul_mat4(mat4 a, mat4 b);
mat4  mat4_look_at(vec3 eye, vec3 target, vec3 up);
mat4  mat4_transpose(mat4 m);
float det3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i);
mat4  mat4_inverse(mat4 M);

#endif
