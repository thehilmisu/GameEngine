#pragma once

#include "renderer/renderer_frontend.h"
#include "renderer/renderer_backend.h"

// Create a cube - a more obvious 3D shape
static const vertex cube_vertices[] = {
    // Front face (gray)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // bottom-right
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-right
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // top-left

    // Back face (gray)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-left
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // top-left
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-right
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // bottom-right

    // Top face (gray)
    // Triangle 1
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // back-left
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // front-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // front-right
    // Triangle 2
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // front-right
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // back-right

    // Bottom face (gray)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // back-right
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // front-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // front-right
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // front-left

    // Right face (gray)
    // Triangle 1
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-back
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // top-back
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-front
    // Triangle 2
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-back
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-front
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // bottom-front

    // Left face (gray)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-back
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},  // bottom-front
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-front
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 0.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}, // bottom-back
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .tex_coords = (vec2){{0.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}},   // top-front
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .tex_coords = (vec2){{1.0f, 1.0f}}, .color = (vec4){{0.5f, 0.5f, 0.5f, 1.0f}}}   // top-back
};
