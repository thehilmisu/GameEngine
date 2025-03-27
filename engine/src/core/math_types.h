#pragma once

#include "definitions.h"

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define MAX and MIN macros if not already defined
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef union vec2 {
    struct {
        f32 x, y;
    };
    f32 data[2];
} vec2;

typedef union vec3 {
    struct {
        f32 x, y, z;
    };
    f32 data[3];
} vec3;

typedef union vec4 {
    struct {
        f32 x, y, z, w;
    };
    f32 data[4];
} vec4; 