#pragma once

#include "../renderer_backend.h"
#include <GL/glew.h>  // GLEW must be included before any OpenGL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stddef.h>  // For NULL
#include "platform/platform.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "core/math_functions.h"

typedef struct opengl_renderer_state {
    struct platform_state* plat_state;
    SDL_Window* window;
    SDL_GLContext gl_context;
    u32 shader_program;
    u32 text_shader_program;  // Added text shader program
    u32 vao;
    u32 vbo;
    u32 text_vao;            // Added text VAO
    u32 text_vbo;            // Added text VBO
    f32 rotation;  // Add rotation state
    render_packet* current_packet;  // Add current packet pointer
    FT_Library ft_library;         // FreeType library instance
} opengl_renderer_state;

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