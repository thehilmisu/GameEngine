#pragma once

#include "../renderer_backend.h"
#include "shaders/shader.h"
#include <GL/glew.h>  // GLEW must be included before any OpenGL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stddef.h>  // For NULL
// Forward declare model struct
typedef struct model model;
#include "platform/platform.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "core/math_functions.h"

// Forward declares
typedef struct FT_LibraryRec_ *FT_Library;

typedef struct opengl_renderer_state {
    struct platform_state* plat_state;
    SDL_Window* window;
    SDL_GLContext gl_context;
    u32 shader_program;            // Standard shader program ID
    u32 text_shader_program;       // Text shader program ID
    u32 vao;
    u32 vbo;
    u32 text_vao;                  // Text VAO
    u32 text_vbo;                  // Text VBO
    f32 rotation;                  // Rotation state
    render_packet* current_packet; // Current packet pointer
    FT_Library ft_library;         // FreeType library instance
    
    // Matrices for rendering
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 model_matrix;
} opengl_renderer_state;

// Matrix helper functions
void create_model_matrix(mat4* matrix, vec3 position, vec3 rotation, vec3 scale);
void create_view_matrix(mat4* matrix, vec3 camera_pos, vec3 camera_rotation);
void create_perspective_matrix(mat4* matrix, float fov_degrees, float aspect_ratio, float near_plane, float far_plane);

b8 opengl_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
void opengl_renderer_backend_shutdown(renderer_backend* backend);
void opengl_renderer_backend_resized(renderer_backend* backend, u16 width, u16 height);
b8 opengl_renderer_backend_begin_frame(renderer_backend* backend, render_packet* packet);
b8 opengl_renderer_backend_end_frame(renderer_backend* backend, render_packet* packet);
b8 opengl_renderer_draw_frame(renderer_backend* backend, render_packet* packet);
// Mesh functions
mesh* opengl_renderer_create_mesh(const vertex* vertices, u32 vertex_count);
void opengl_renderer_destroy_mesh(mesh* m);
void opengl_renderer_draw_mesh(mesh* m);
mesh* opengl_renderer_get_mesh(u32 mesh_id);

// Font functions
font* opengl_renderer_create_font(const char* font_path, u32 font_size);
font* opengl_renderer_create_fallback_font(u32 font_size);
void opengl_renderer_destroy_font(font* f);
void opengl_renderer_draw_text(font* f, const char* text, vec2 position, vec4 color, f32 scale); 

// Model functions
model* opengl_renderer_create_model(const char* model_path);
void opengl_renderer_destroy_model(model* m);
void opengl_renderer_draw_model(model* m);