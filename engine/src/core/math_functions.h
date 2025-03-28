#pragma once

#include "math_types.h"
#include <math.h>

// Matrix 4x4 type
typedef union mat4 {
    f32 elements[16];
    f32 data[16];  // Alias for elements for compatibility
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };
} mat4;

// Create an identity matrix
static inline mat4 mat4_identity() {
    mat4 result = {0};
    result.m00 = 1.0f;
    result.m11 = 1.0f;
    result.m22 = 1.0f;
    result.m33 = 1.0f;
    return result;
}

// Create an orthographic projection matrix
static inline mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
    mat4 result = mat4_identity();

    result.m00 = 2.0f / (right - left);
    result.m11 = 2.0f / (top - bottom);
    result.m22 = -2.0f / (far - near);
    result.m03 = -(right + left) / (right - left);
    result.m13 = -(top + bottom) / (top - bottom);
    result.m23 = -(far + near) / (far - near);

    return result;
}

// Create a perspective projection matrix
static inline mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near, f32 far) {
    mat4 result = {0};
    f32 tan_half_fov = tanf(fov_radians / 2.0f);

    result.m00 = 1.0f / (aspect_ratio * tan_half_fov);
    result.m11 = 1.0f / tan_half_fov;
    result.m22 = -(far + near) / (far - near);
    result.m23 = -(2.0f * far * near) / (far - near);
    result.m32 = -1.0f;

    return result;
}

// Create a translation matrix
static inline mat4 mat4_translation(vec3 translation) {
    mat4 result = mat4_identity();
    result.m03 = translation.x;
    result.m13 = translation.y;
    result.m23 = translation.z;
    return result;
}

// Create a rotation matrix from Euler angles (in radians)
static inline mat4 mat4_rotation(vec3 rotation) {
    mat4 result = mat4_identity();

    f32 cx = cosf(rotation.x);
    f32 cy = cosf(rotation.y);
    f32 cz = cosf(rotation.z);
    f32 sx = sinf(rotation.x);
    f32 sy = sinf(rotation.y);
    f32 sz = sinf(rotation.z);

    result.m00 = cy * cz;
    result.m01 = -cy * sz;
    result.m02 = sy;

    result.m10 = cx * sz + cz * sx * sy;
    result.m11 = cx * cz - sx * sy * sz;
    result.m12 = -cy * sx;

    result.m20 = sx * sz - cx * cz * sy;
    result.m21 = cz * sx + cx * sy * sz;
    result.m22 = cx * cy;

    return result;
}

// Matrix multiplication
static inline mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 result = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            f32 sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += a.elements[i * 4 + k] * b.elements[k * 4 + j];
            }
            result.elements[i * 4 + j] = sum;
        }
    }
    return result;
} 