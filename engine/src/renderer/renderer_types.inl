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
    vec2 tex_coords;  // Texture coordinates
    vec4 color;
} vertex;

// Text vertex data structure
typedef struct text_vertex {
    vec3 position;  // Position for text (x, y, z)
    vec2 tex_coords;  // Texture coordinates for font atlas
    vec4 color;  // Color of the text
} text_vertex;

// Font character structure
typedef struct font_character {
    u32 texture_id;  // ID handle of the glyph texture
    u32 width;       // Width of glyph
    u32 height;      // Height of glyph
    i32 bearing_x;   // Offset from baseline to left of glyph
    i32 bearing_y;   // Offset from baseline to top of glyph
    u32 advance;     // Horizontal offset to advance to next glyph
} font_character;

// Font data structure
typedef struct font {
    u32 id;
    font_character characters[128];  // Use the font_character type
    u32 shader_id;
    u32 shader_program;
    u32 vao;
    u32 vbo;
} font;

// Texture data structure
typedef struct texture {
    u32 id;                // OpenGL texture ID
    u32 width;             // Width of the texture
    u32 height;            // Height of the texture
    u32 channels;          // Number of channels (1=R, 2=RG, 3=RGB, 4=RGBA)
    char path[256];        // Path to the texture file
    void* data;            // Raw texture data (can be NULL after upload to GPU)
} texture;

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

// Command structures
typedef struct text_command {
    const char* text;
    u32 text_id;
    vec2 position;
    vec4 color;
    f32 scale;
    font* font;
} text_command;

typedef struct mesh_command {
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec4 color;
    mesh* mesh;
} mesh_command;

// Model data structure
typedef struct model {
    u32 id;                // Unique model ID
    u32 vertex_count;      // Number of vertices
    u32 index_count;       // Number of indices (if indexed)
    vertex* vertices;      // Array of vertices
    u32* indices;          // Array of indices (if indexed)
    b8 is_indexed;         // Whether the model uses indexed geometry
    char name[64];         // Model name
    mesh* mesh;
    texture* texture;      // Model texture
} model;

typedef struct model_command {
    u32 model_id;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec4 color;
    model* model;
} model_command;

// Render packet structure
typedef struct render_packet {
    f32 delta_time;  // Time since last frame
    struct {
        text_command* commands;
        u32 count;
    } text_commands;
    struct {
        mesh_command* commands;
        u32 count;
    } mesh_commands;
    struct {
        model_command* commands;
        u32 count;
    } model_commands;
    vec3 camera_position;
    vec3 camera_rotation;
} render_packet;
 
typedef struct renderer_backend {
    struct platform_state* plat_state;
    void* internal_state;
    u64 frame_number;
    font* default_font;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
    b8 (*begin_frame)(struct renderer_backend* backend, render_packet* packet);
    b8 (*end_frame)(struct renderer_backend* backend, render_packet* packet);
    b8 (*draw_frame)(struct renderer_backend* backend, render_packet* packet);

    // Mesh functions
    mesh* (*create_mesh)(const vertex* vertices, u32 vertex_count);
    void (*destroy_mesh)(mesh* m);
    void (*draw_mesh)(mesh* m);
    mesh* (*get_mesh)(u32 mesh_id);

    // Model functions
    model* (*create_model)(const char* model_path);
    void (*destroy_model)(model* m);
    void (*draw_model)(model* m);

    // Text functions
    font* (*create_font)(const char* font_path, u32 font_size);
    font* (*create_fallback_font)(u32 font_size);
    void (*destroy_font)(font* f);
    void (*draw_text)(font* f, const char* text, vec2 position, vec4 color, f32 scale);
} renderer_backend;
