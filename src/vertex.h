#ifndef VERTEX_H
#define VERTEX_H

#include "c3m.h"

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    u32 color;
} vertex_t;

#endif // VERTEX_H
