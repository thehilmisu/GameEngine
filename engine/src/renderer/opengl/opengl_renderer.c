#include "opengl_renderer.h"
#include "../renderer_types.inl"
#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"
#include "platform/platform.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>

// Shader source code
static const char* vertex_shader_source = 
    "#version 450 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "uniform float rotation;\n"
    "out vec4 vertexColor;\n"
    "void main()\n"
    "{\n"
    "   float cos_rot = cos(rotation);\n"
    "   float sin_rot = sin(rotation);\n"
    "   vec3 rotated_pos;\n"
    "   rotated_pos.x = aPos.x * cos_rot - aPos.y * sin_rot;\n"
    "   rotated_pos.y = aPos.x * sin_rot + aPos.y * cos_rot;\n"
    "   rotated_pos.z = aPos.z;\n"
    "   gl_Position = vec4(rotated_pos.x, rotated_pos.y, rotated_pos.z, 1.0);\n"
    "   vertexColor = aColor;\n"
    "}\0";

static const char* fragment_shader_source = 
    "#version 450 core\n"
    "in vec4 vertexColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vertexColor;\n"
    "}\0";

typedef struct opengl_renderer_backend {
    SDL_GLContext context;
    u32 shader_program;
    u32 vao;
    u32 vbo;
} opengl_renderer_backend;

static opengl_renderer_state* global_renderer_state = NULL;

static u32 next_mesh_id = 1;

b8 opengl_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    backend->plat_state = plat_state;
    
    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(plat_state->window_handle);
    if (!gl_context) {
        ERROR("Failed to create OpenGL context! SDL Error: %s", SDL_GetError());
        return FALSE;
    }

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        ERROR("Failed to initialize GLEW: %s", glewGetErrorString(err));
        return FALSE;
    }

    // Create and allocate the internal state
    opengl_renderer_state* state = kallocate(sizeof(opengl_renderer_state), MEMORY_TAG_RENDERER);
    state->gl_context = gl_context;
    state->current_packet = NULL;

    // Create and compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    // Check vertex shader compilation
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        ERROR("Vertex shader compilation failed: %s", info_log);
        return FALSE;
    }

    // Create and compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // Check fragment shader compilation
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        ERROR("Fragment shader compilation failed: %s", info_log);
        return FALSE;
    }

    // Create shader program
    state->shader_program = glCreateProgram();
    glAttachShader(state->shader_program, vertex_shader);
    glAttachShader(state->shader_program, fragment_shader);
    glLinkProgram(state->shader_program);

    // Check program linking
    glGetProgramiv(state->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(state->shader_program, 512, NULL, info_log);
        ERROR("Shader program linking failed: %s", info_log);
        return FALSE;
    }

    // Delete shaders as they're linked into our program and no longer necessary
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    backend->internal_state = state;
    global_renderer_state = state;
    INFO("OpenGL renderer initialized successfully");
    return TRUE;
}

void opengl_renderer_backend_shutdown(renderer_backend* backend) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    
    if (state) {
        // Make sure we have the current context
        SDL_GL_MakeCurrent(backend->plat_state->window_handle, state->gl_context);
        
        // Delete shader program
        glDeleteProgram(state->shader_program);
        
        // Delete OpenGL context
        SDL_GL_DeleteContext(state->gl_context);
        
        // Free the internal state
        kfree(state, sizeof(opengl_renderer_state), MEMORY_TAG_RENDERER);
        backend->internal_state = 0;
        global_renderer_state = NULL;
    }
}

void opengl_renderer_backend_resized(renderer_backend* backend, u16 width, u16 height) {
    glViewport(0, 0, width, height);
}

b8 opengl_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    if (!state) {
        ERROR("No renderer state!");
        return FALSE;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return TRUE;
}

b8 opengl_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    if (!state) {
        ERROR("No renderer state!");
        return FALSE;
    }

    SDL_GL_SwapWindow(backend->plat_state->window_handle);
    return TRUE;
}

mesh* opengl_renderer_create_mesh(const vertex* vertices, u32 vertex_count) {
    if (!vertices || vertex_count == 0) {
        ERROR("Invalid vertices or vertex count!");
        return NULL;
    }

    mesh* m = kallocate(sizeof(mesh), MEMORY_TAG_RENDERER);
    m->id = next_mesh_id++;
    m->vertex_count = vertex_count;
    m->vertex_buffer_size = vertex_count * sizeof(vertex);
    m->vertices = kallocate(m->vertex_buffer_size, MEMORY_TAG_RENDERER);
    kcopy_memory(m->vertices, vertices, m->vertex_buffer_size);

    // Generate and bind VAO
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // Generate and bind VBO
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_buffer_size, m->vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(1);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        ERROR("OpenGL error while creating mesh: %u", error);
        opengl_renderer_destroy_mesh(m);
        return NULL;
    }

    INFO("Created mesh with ID: %u, VAO: %u, VBO: %u", m->id, m->vao, m->vbo);
    return m;
}

void opengl_renderer_destroy_mesh(mesh* m) {
    if (m) {
        if (m->vertices) {
            kfree(m->vertices, m->vertex_buffer_size, MEMORY_TAG_RENDERER);
            m->vertices = 0;
        }
        if (m->vao != 0) {
            glDeleteVertexArrays(1, &m->vao);
            m->vao = 0;
        }
        if (m->vbo != 0) {
            glDeleteBuffers(1, &m->vbo);
            m->vbo = 0;
        }
        kfree(m, sizeof(mesh), MEMORY_TAG_RENDERER);
    }
}

void opengl_renderer_draw_mesh(mesh* m) {
    if (!m || !m->vao) {
        ERROR("Invalid mesh or VAO!");
        return;
    }

    opengl_renderer_state* state = global_renderer_state;
    if (!state) {
        ERROR("No renderer state!");
        return;
    }

    // Use shader program
    glUseProgram(state->shader_program);

    // Set rotation uniform
    GLint rotation_loc = glGetUniformLocation(state->shader_program, "rotation");
    if (rotation_loc != -1) {
        glUniform1f(rotation_loc, state->current_packet ? state->current_packet->rotation : 0.0f);
    }

    // Bind VAO and draw
    glBindVertexArray(m->vao);
    glDrawArrays(GL_TRIANGLES, 0, m->vertex_count);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        ERROR("OpenGL error while drawing mesh: %u", error);
    }
}

// Add function to update rotation
void opengl_renderer_update_rotation(f32 delta_time) {
    if (global_renderer_state) {
        global_renderer_state->rotation += delta_time * 2.0f;  // Rotate 2 radians per second
    }
} 