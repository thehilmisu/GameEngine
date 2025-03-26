#pragma once

#include "definitions.h"

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