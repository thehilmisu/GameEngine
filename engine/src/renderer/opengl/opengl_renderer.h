#pragma once

#include "../renderer_backend.h"
#include <GL/glew.h>  // GLEW must be included before any OpenGL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stddef.h>  // For NULL
#include "platform/platform.h"

typedef struct opengl_renderer_state {
    SDL_GLContext gl_context;
    u32 shader_program;
    u32 vao;
    u32 vbo;
    f32 rotation;  // Add rotation state
    render_packet* current_packet;  // Add current packet pointer
} opengl_renderer_state;

b8 opengl_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
void opengl_renderer_backend_shutdown(renderer_backend* backend);
void opengl_renderer_backend_resized(renderer_backend* backend, u16 width, u16 height);
b8 opengl_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time);
b8 opengl_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time);

// Mesh functions
mesh* opengl_renderer_create_mesh(const vertex* vertices, u32 vertex_count);
void opengl_renderer_destroy_mesh(mesh* m);
void opengl_renderer_draw_mesh(mesh* m);
void opengl_renderer_update_rotation(f32 delta_time); 