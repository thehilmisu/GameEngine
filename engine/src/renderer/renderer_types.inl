#pragma once
 
#include "definitions.h"
#include "core/math_types.h"
#include <GL/glew.h>
 
typedef enum renderer_backend_type {
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

// Vertex data structure
typedef struct vertex {
    vec3 position;
    vec4 color;
} vertex;

// Mesh data structure
typedef struct mesh {
    u32 id;
    u32 vertex_count;
    u32 vertex_buffer_size;
    vertex* vertices;
    // OpenGL-specific data
    GLuint vao;
    GLuint vbo;
} mesh;
 
typedef struct renderer_backend {
    struct platform_state* plat_state;
    void* internal_state;
    u64 frame_number;
 
    b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
 
    void (*shutdown)(struct renderer_backend* backend);
 
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
 
    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);    

    // Mesh functions
    mesh* (*create_mesh)(const vertex* vertices, u32 vertex_count);
    void (*destroy_mesh)(mesh* m);
    void (*draw_mesh)(mesh* m);
} renderer_backend;
 
typedef struct render_packet {
    f32 delta_time;
    mesh* meshes;
    u32 mesh_count;
    f32 rotation;
} render_packet;
