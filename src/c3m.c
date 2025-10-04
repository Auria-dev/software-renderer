#include "c3m.h"

vec2 vec2_new(float x, float y) {
  return (vec2) {x,y};
}

vec2 vec2_add(vec2 a, vec2 b) {
  return (vec2) {a.x+b.x, a.y+b.y};
}

vec2 vec2_sub(vec2 a, vec2 b) {
  return (vec2) {a.x-b.x, a.y-b.y};
}

vec2 vec2_mul(vec2 a, vec2 b) {
  return (vec2) {a.x*b.x, a.y*b.y};
}

vec2 vec2_div(vec2 a, vec2 b) {
  return (vec2) {a.x/b.x, a.y/b.y};
}

vec2 vec2_scale(vec2 a, float s) {
  return (vec2) {a.x*s, a.y*s};
}

vec2 vec2_normalize(vec2 v) {
  float l = vec2_len(v);
  return (l > 0.0f) ? vec2_scale(v, 1.0f / l) : v;
}

bool vec2_eq(vec2 a, vec2 b) {
  return (a.x == b.x && a.y == b.y);
}

vec2 vec2_clone(vec2 *v) {
  return vec2_new(v->x, v->y);
}

vec2 vec2_lerp(vec2 a, vec2 b, float t) {
  return vec2_add(vec2_scale(a, 1.0f - t), vec2_scale(b, t));
}

float vec2_dot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}

float vec2_len(vec2 v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

float vec2_len_sq(vec2 v) {
  return v.x * v.x + v.y * v.y;
}

bool vec3_eq(vec3 a, vec3 b) {
  return (a.x == b.x && a.y == b.y && a.z == b.z);
} 

vec3 vec3_new(float x, float y, float z) {
  return (vec3) {x,y,z};
}

vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3) {
    a.x + b.x, a.y + b.y, a.z + b.z
  };
}

vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3) {
    a.x - b.x, a.y - b.y, a.z - b.z
  };
}

vec3 vec3_scale(vec3 v, float s) {
  return (vec3) {
    v.x * s, v.y * s, v.z * s
  };
}

vec3 vec3_div(vec3 v, float s) {
  return (vec3) {
    v.x / s, v.y / s, v.z / s
  };
}

float vec3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 vec3_cross(vec3 a, vec3 b) {
  return (vec3) {
    a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x
  };
}

float vec3_len_sq(vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vec3_len(vec3 v) {
  return sqrtf(vec3_len_sq(v));
}

vec3 vec3_normalize(vec3 v) {
  float l = vec3_len(v);
  return (l > 0.0f) ? vec3_scale(v, 1.0f / l) : v;
}

vec3 vec3_lerp(vec3 a, vec3 b, float t) {
  return vec3_add(vec3_scale(a, 1.0f - t), vec3_scale(b, t));
}

vec3 vec3_reflect(vec3 v, vec3 n) {
  return vec3_sub(v, vec3_scale(n, 2.0f * vec3_dot(v, n)));
}

vec3 vec3_negate(vec3 v) {
  return (vec3) {
    -v.x, -v.y, -v.z
  };
}

vec3 vec3_zero() {
  return vec3_new(0, 0, 0);
}

vec3 vec3_left() {
  return vec3_new(1, 0, 0);
}

vec3 vec3_up() {
  return vec3_new(0, 1, 0);
}

vec3 vec3_forward() {
  return vec3_new(0, 0, 1);
}

vec3 vec3_clone(vec3 * v) {
  return vec3_new(v -> x, v -> y, v -> z);
}

vec3 vec3_min(vec3 a, vec3 b) {
  vec3 r;
  r.x = a.x < b.x ? a.x : b.x;
  r.y = a.y < b.y ? a.y : b.y;
  r.z = a.z < b.z ? a.z : b.z;
  return r;
}

vec3 vec3_max(vec3 a, vec3 b) {
  vec3 r;
  r.x = a.x > b.x ? a.x : b.x;
  r.y = a.y > b.y ? a.y : b.y;
  r.z = a.z > b.z ? a.z : b.z;
  return r;
}

vec3 vec3_orthogonal(vec3 v) {
  if (fabsf(v.x) > fabsf(v.z)) {
    return vec3_normalize((vec3) {
      -v.y, v.x, 0.0f
    });
  } else {
    return vec3_normalize((vec3) {
      0.0f, -v.z, v.y
    });
  }
} // returns any unit vector orthogonal to v

vec4 vec4_new(float x, float y, float z, float w) {
  return (vec4) {x,y,z,w};
}

vec4 vec4_add(vec4 a, vec4 b) {
  return (vec4) {
    a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w
  };
}

vec4 vec4_sub(vec4 a, vec4 b) {
  return (vec4) {
    a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w
  };
}

vec4 vec4_scale(vec4 v, float s) {
  return (vec4) {
    v.x * s, v.y * s, v.z * s, v.w * s
  };
}

float vec4_dot(vec4 a, vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float vec4_len_sq(vec4 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

float vec4_len(vec4 v) {
  return sqrtf(vec4_len_sq(v));
}

vec4 vec4_normalize(vec4 v) {
  float l = vec4_len(v);
  return (l > 0.0f) ? vec4_scale(v, 1.0f / l) : v;
}

vec4 vec4_lerp(vec4 a, vec4 b, float t) {
  return vec4_add(vec4_scale(a, 1.0f - t), vec4_scale(b, t));
}

quat quat_new(float x, float y, float z, float w) {
  return (quat) {x,y,z,w};
}

quat quat_identity() {
  return (quat) {
    0.0f, 0.0f, 0.0f, 1.0f
  };
}

quat quat_add(quat a, quat b) {
  return (quat) {
    a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w
  };
}

quat quat_sub(quat a, quat b) {
  return (quat) {
    a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w
  };
}

quat quat_scale(quat v, float s) {
  return (quat) {
    v.x * s, v.y * s, v.z * s, v.w * s
  };
}

quat quat_mul(quat p, quat q) {
  return (quat) {
    p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
      p.w * q.y - p.x * q.z + p.y * q.w + p.z * q.x,
      p.w * q.z + p.x * q.y - p.y * q.x + p.z * q.w,
      p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z
  };
}

quat quat_conjugate(quat q) {
  return (quat) {
    -q.x, -q.y, -q.z, q.w
  };
}

quat quat_inverse(quat q) {
  return vec4_scale(quat_conjugate(q), 1.0f / vec4_len_sq(q));
}

vec3 quat_rotate_vec3(quat q, vec3 v) {
  vec3 u = (vec3) {
    q.x, q.y, q.z
  };
  vec3 t = vec3_cross(u, v);
  t = vec3_scale(t, 2.0f);
  vec3 v_out = vec3_add(v, vec3_add(vec3_scale(t, q.w), vec3_cross(u, t)));
  return v_out;
}

quat quat_from_axis_angle(vec3 axis, float angle_rad) {
  float half_angle = angle_rad * 0.5f;
  float s = sinf(half_angle);
  vec3 n_axis = vec3_normalize(axis);
  return (quat) {
    n_axis.x * s, n_axis.y * s, n_axis.z * s, cosf(half_angle)
  };
}

quat quat_slerp(quat a, quat b, float t) {
  float cos_half_theta = vec4_dot(a, b);
  if (fabsf(cos_half_theta) >= 1.0f) return a;

  if (cos_half_theta < 0.0f) {
    b = vec4_scale(b, -1.0f);
    cos_half_theta = -cos_half_theta;
  }

  float half_theta = acosf(cos_half_theta);
  float sin_half_theta = sqrtf(1.0f - cos_half_theta * cos_half_theta);

  if (fabsf(sin_half_theta) < EPS) return vec4_lerp(a, b, t);

  float ratio_a = sinf((1.0f - t) * half_theta) / sin_half_theta;
  float ratio_b = sinf(t * half_theta) / sin_half_theta;

  return vec4_add(vec4_scale(a, ratio_a), vec4_scale(b, ratio_b));
}

// finds shortest rotation U should take to point in the same direction as V
quat quat_from_to(vec3 u, vec3 v) {
  float dot = vec3_dot(u, v);
  if (dot >= 1.0f) {
    // vectors are the same
    return quat_new(0, 0, 0, 1);
  }
  if (dot <= -1.0f) {
    // vectors are opposite; need any orthogonal axis
    vec3 orth = fabsf(u.x) < 0.1f ? vec3_new(1, 0, 0) : vec3_new(0, 1, 0);
    vec3 axis = vec3_normalize(vec3_cross(u, orth));
    return quat_from_axis_angle(axis, M_PI); // 180° around orthogonal axis
  }
  vec3 axis = vec3_cross(u, v);
  float s = sqrtf((1.0f + dot) * 2.0f);
  float invs = 1.0f / s;
  return quat_new(
    axis.x * invs,
    axis.y * invs,
    axis.z * invs,
    s * 0.5f
  );
}

float quat_length_sq(quat q) {
  return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

float quat_length(quat q) {
  return sqrtf(quat_length_sq(q));
}

quat quat_normalize(quat q) {
  float len = quat_length(q);
  if (len > EPS) {
    float inv = 1.0f / len;
    q.x *= inv;
    q.y *= inv;
    q.z *= inv;
    q.w *= inv;
    return q;
  }
  return quat_identity();
}

quat quat_from_vec3_angle(vec3 v, float half_angle) {
  float len = vec3_len(v);
  if (len < EPS) return quat_identity();
  float ha = half_angle * len;
  vec3 axis = vec3_scale(v, 1.0f / len);
  return quat_from_axis_angle(axis, ha);
}

mat3 quat_to_mat3(quat q) {
  mat3 result;

  float x = q.x, y = q.y, z = q.z, w = q.w;
  float xx = x * x, yy = y * y, zz = z * z;
  float xy = x * y, xz = x * z, yz = y * z;
  float wx = w * x, wy = w * y, wz = w * z;

  result.m[0][0] = 1.0f - 2.0f * (yy + zz);
  result.m[0][1] = 2.0f * (xy - wz);
  result.m[0][2] = 2.0f * (xz + wy);

  result.m[1][0] = 2.0f * (xy + wz);
  result.m[1][1] = 1.0f - 2.0f * (xx + zz);
  result.m[1][2] = 2.0f * (yz - wx);

  result.m[2][0] = 2.0f * (xz - wy);
  result.m[2][1] = 2.0f * (yz + wx);
  result.m[2][2] = 1.0f - 2.0f * (xx + yy);

  return result;
}



mat3 mat3_identity(void) {
    return (mat3) {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
}

mat3 mat3_zero(void) {
    return (mat3) {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
}

mat3 mat3_init(float a, float b, float c, float d, float e, float f, float g, float h, float i) {
    return (mat3) {{{a,b,c},{d,e,f},{g,h,i}}};
}

vec3 mat3_mul_vec3(mat3 m, vec3 v) {
    return (vec3) {
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z
    };
}

mat3 mat3_mul_mat3(mat3 a, mat3 b) {
    mat3 result = {0};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

mat3 mat3_transpose(mat3 m) {
    mat3 result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = m.m[j][i];
        }
    }
    return result;
}

mat3 mat3_scale(mat3 m, float s) {
    mat3 result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = m.m[i][j]*s;
        }
    }
    return result;
}

mat3 mat3_inverse_diagonal(mat3 m) {
    mat3 result = {0};
    if (fabsf(m.m[0][0]) > EPS) result.m[0][0] = 1.0f / m.m[0][0];
    if (fabsf(m.m[1][1]) > EPS) result.m[1][1] = 1.0f / m.m[1][1];
    if (fabsf(m.m[2][2]) > EPS) result.m[2][2] = 1.0f / m.m[2][2];

    if (fabsf(m.m[0][1]) > EPS|| fabsf(m.m[0][2]) > EPS||
        fabsf(m.m[1][0]) > EPS|| fabsf(m.m[1][2]) > EPS||
        fabsf(m.m[2][0]) > EPS|| fabsf(m.m[2][1]) > EPS) {
        return mat3_identity();
    }
    return result;
}

mat3 mat3_inverse(mat3 m) {
    // cofactors
    float c00 =  m.m[1][1]*m.m[2][2] - m.m[1][2]*m.m[2][1];
    float c01 =-(m.m[1][0]*m.m[2][2] - m.m[1][2]*m.m[2][0]);
    float c02 =  m.m[1][0]*m.m[2][1] - m.m[1][1]*m.m[2][0];

    float c10 =-(m.m[0][1]*m.m[2][2] - m.m[0][2]*m.m[2][1]);
    float c11 =  m.m[0][0]*m.m[2][2] - m.m[0][2]*m.m[2][0];
    float c12 =-(m.m[0][0]*m.m[2][1] - m.m[0][1]*m.m[2][0]);

    float c20 =  m.m[0][1]*m.m[1][2] - m.m[0][2]*m.m[1][1];
    float c21 =-(m.m[0][0]*m.m[1][2] - m.m[0][2]*m.m[1][0]);
    float c22 =  m.m[0][0]*m.m[1][1] - m.m[0][1]*m.m[1][0];

    // determinant
    float det = m.m[0][0]*c00 + m.m[0][1]*c01 + m.m[0][2]*c02;
    if (fabsf(det) < EPS) { // non-invertible
        return mat3_identity();
    }

    float inv_det = 1.0f / det;

    mat3 inv;
    inv.m[0][0] = c00 * inv_det;
    inv.m[0][1] = c10 * inv_det;
    inv.m[0][2] = c20 * inv_det;

    inv.m[1][0] = c01 * inv_det;
    inv.m[1][1] = c11 * inv_det;
    inv.m[1][2] = c21 * inv_det;

    inv.m[2][0] = c02 * inv_det;
    inv.m[2][1] = c12 * inv_det;
    inv.m[2][2] = c22 * inv_det;

    return inv;
}

mat4 mat4_identity(void) {
    mat4 m = {{
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 }
    }};
    return m;
}

mat4 mat4_from_pos_quat(vec3 v, quat q) {
    mat3 r = quat_to_mat3(q);
    mat4 out = mat4_identity();
    out.m[0][0] = r.m[0][0]; out.m[0][1] = r.m[0][1]; out.m[0][2] = r.m[0][2]; out.m[0][3] = v.x;
    out.m[1][0] = r.m[1][0]; out.m[1][1] = r.m[1][1]; out.m[1][2] = r.m[1][2]; out.m[1][3] = v.y;
    out.m[2][0] = r.m[2][0]; out.m[2][1] = r.m[2][1]; out.m[2][2] = r.m[2][2]; out.m[2][3] = v.z;
    return out;
}

mat4 mat4_make_scale(float sx, float sy, float sz) {
    mat4 m = mat4_identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

mat4 mat4_make_translation(float tx, float ty, float tz) {
    mat4 m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

mat4 mat4_make_perspective(float fov, float aspect, float znear, float zfar) {
    mat4 m = {{{ 0 }}};
    m.m[0][0] =        aspect * (1/tan(fov/2.0));
    m.m[1][1] =                 (1/tan(fov/2.0));
    m.m[2][2] =   zfar        /     (zfar-znear);
    m.m[2][3] = (-zfar*znear) /     (zfar-znear);
    m.m[3][2] =                              1.0;
    return m;
}

vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 r;
    r.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    r.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    r.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    r.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
    return r;
}

vec4 mat4_mul_vec4_fast(mat4 m, vec4 v) {
    __m128 row0 = _mm_loadu_ps(&m.m[0][0]);
    __m128 row1 = _mm_loadu_ps(&m.m[1][0]);
    __m128 row2 = _mm_loadu_ps(&m.m[2][0]);
    __m128 row3 = _mm_loadu_ps(&m.m[3][0]);
    __m128 v_reg = _mm_loadu_ps(&v.x);
    
    row0 = _mm_mul_ps(row0, v_reg);
    row1 = _mm_mul_ps(row1, v_reg);
    row2 = _mm_mul_ps(row2, v_reg);
    row3 = _mm_mul_ps(row3, v_reg);
    
    __m128 hsum0 = _mm_hadd_ps(row0, row1);
    __m128 hsum1 = _mm_hadd_ps(row2, row3);
    __m128 result = _mm_hadd_ps(hsum0, hsum1);
    
    vec4 r;
    _mm_storeu_ps(&r.x, result);
    return r;
}

vec4 mat4_mul_vec4_project(mat4 mat_proj, vec4 v)  {
    vec4 r = mat4_mul_vec4_fast(mat_proj, v);
    if (r.w <= 0) return r;
    
    float rw = 1.0/r.w;
    r.x *= rw;
    r.y *= rw;
    r.z *= rw;

    return r;
}

mat4 mat4_make_rotation_x(float a) {
    float c = cos(a);
    float s = sin(a);
    mat4 m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

mat4 mat4_make_rotation_y(float a) {
    float c = cos(a);
    float s = sin(a);
    mat4 m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

mat4 mat4_make_rotation_z(float a) {
    float c = cos(a);
    float s = sin(a);
    mat4 m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return m;
}

vec3 mat4_mul_vec3(mat4 m, vec3 v) {
    vec3 r;
    r.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z;
    r.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z;
    r.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z;
    return r;
}

mat4 mat4_mul_mat4(mat4 a, mat4 b) {
    mat4 m;
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            m.m[i][j] = a.m[i][0]*b.m[0][j] + a.m[i][1]*b.m[1][j] + a.m[i][2]*b.m[2][j] + a.m[i][3]*b.m[3][j];
        }
    }
    return m;
}

mat4 mat4_look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 z = vec3_normalize(vec3_sub(target, eye));
    vec3 x = vec3_normalize(vec3_cross(up, z));
    vec3 y = vec3_cross(z, x); // y is already normalized

    mat4 view_matrix = {{
        {x.x, x.y, x.z, -vec3_dot(x,eye)},
        {y.x, y.y, y.z, -vec3_dot(y,eye)},
        {z.x, z.y, z.z, -vec3_dot(z,eye)},
        {  0,   0,   0,                1}
    }};

    return view_matrix;
}

mat4 mat4_transpose(mat4 m) {
    mat4 t;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            t.m[i][j] = m.m[j][i];
        }
    }
    return t;
}

float det3x3(float a, float b, float c,
                           float d, float e, float f,
                           float g, float h, float i) {
    return a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
}

mat4 mat4_inverse(mat4 M) {
    // Aliases for matrix elements
    float m00 = M.m[0][0], m01 = M.m[0][1], m02 = M.m[0][2], m03 = M.m[0][3];
    float m10 = M.m[1][0], m11 = M.m[1][1], m12 = M.m[1][2], m13 = M.m[1][3];
    float m20 = M.m[2][0], m21 = M.m[2][1], m22 = M.m[2][2], m23 = M.m[2][3];
    float m30 = M.m[3][0], m31 = M.m[3][1], m32 = M.m[3][2], m33 = M.m[3][3];

    // Compute all 16 minors
    float minor00 = det3x3(m11, m12, m13, m21, m22, m23, m31, m32, m33);
    float minor01 = det3x3(m10, m12, m13, m20, m22, m23, m30, m32, m33);
    float minor02 = det3x3(m10, m11, m13, m20, m21, m23, m30, m31, m33);
    float minor03 = det3x3(m10, m11, m12, m20, m21, m22, m30, m31, m32);

    float minor10 = det3x3(m01, m02, m03, m21, m22, m23, m31, m32, m33);
    float minor11 = det3x3(m00, m02, m03, m20, m22, m23, m30, m32, m33);
    float minor12 = det3x3(m00, m01, m03, m20, m21, m23, m30, m31, m33);
    float minor13 = det3x3(m00, m01, m02, m20, m21, m22, m30, m31, m32);

    float minor20 = det3x3(m01, m02, m03, m11, m12, m13, m31, m32, m33);
    float minor21 = det3x3(m00, m02, m03, m10, m12, m13, m30, m32, m33);
    float minor22 = det3x3(m00, m01, m03, m10, m11, m13, m30, m31, m33);
    float minor23 = det3x3(m00, m01, m02, m10, m11, m12, m30, m31, m32);

    float minor30 = det3x3(m01, m02, m03, m11, m12, m13, m21, m22, m23);
    float minor31 = det3x3(m00, m02, m03, m10, m12, m13, m20, m22, m23);
    float minor32 = det3x3(m00, m01, m03, m10, m11, m13, m20, m21, m23);
    float minor33 = det3x3(m00, m01, m02, m10, m11, m12, m20, m21, m22);

    // Build cofactor matrix with alternating signs
    float cof00 =  minor00;
    float cof01 = -minor01;
    float cof02 =  minor02;
    float cof03 = -minor03;

    float cof10 = -minor10;
    float cof11 =  minor11;
    float cof12 = -minor12;
    float cof13 =  minor13;

    float cof20 =  minor20;
    float cof21 = -minor21;
    float cof22 =  minor22;
    float cof23 = -minor23;

    float cof30 = -minor30;
    float cof31 =  minor31;
    float cof32 = -minor32;
    float cof33 =  minor33;

    // Compute determinant using first row
    float det = m00 * cof00 + m01 * cof01 + m02 * cof02 + m03 * cof03;

    // Check for invertibility
    if (fabsf(det) < EPS) {
        return mat4_identity();
    }

    float inv_det = 1.0f / det;

    // Build inverse by scaling the adjugate (transposed cofactors)
    mat4 inv;
    inv.m[0][0] = cof00 * inv_det;
    inv.m[0][1] = cof10 * inv_det;
    inv.m[0][2] = cof20 * inv_det;
    inv.m[0][3] = cof30 * inv_det;

    inv.m[1][0] = cof01 * inv_det;
    inv.m[1][1] = cof11 * inv_det;
    inv.m[1][2] = cof21 * inv_det;
    inv.m[1][3] = cof31 * inv_det;

    inv.m[2][0] = cof02 * inv_det;
    inv.m[2][1] = cof12 * inv_det;
    inv.m[2][2] = cof22 * inv_det;
    inv.m[2][3] = cof32 * inv_det;

    inv.m[3][0] = cof03 * inv_det;
    inv.m[3][1] = cof13 * inv_det;
    inv.m[3][2] = cof23 * inv_det;
    inv.m[3][3] = cof33 * inv_det;

    return inv;
}
