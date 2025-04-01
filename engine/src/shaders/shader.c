#include "shader.h"
#include "core/logger.h"
#include "core/file_operations.h"
#include "core/kmemory.h"
#include <GL/glew.h>

// Forward declare internal functions
static void check_shader_error(u32 shader, const char* type);
static void check_program_error(u32 program);

shader_program shader_create_from_source(const char* vertex_source, const char* fragment_source) {
    shader_program program = {0};

    // Create and compile vertex shader
    program.vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(program.vertex_shader_id, 1, &vertex_source, NULL);
    glCompileShader(program.vertex_shader_id);
    check_shader_error(program.vertex_shader_id, "vertex");

    // Create and compile fragment shader
    program.fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(program.fragment_shader_id, 1, &fragment_source, NULL);
    glCompileShader(program.fragment_shader_id);
    check_shader_error(program.fragment_shader_id, "fragment");

    // Create shader program and link
    program.program_id = glCreateProgram();
    glAttachShader(program.program_id, program.vertex_shader_id);
    glAttachShader(program.program_id, program.fragment_shader_id);
    glLinkProgram(program.program_id);
    check_program_error(program.program_id);

    return program;
}

shader_program shader_create_from_files(const char* vertex_path, const char* fragment_path) {
    // Default result with zeroed IDs
    shader_program program = {0};
    
    // Read vertex shader from file
    char* vertex_source = NULL;
    u64 vertex_source_size = 0;
    if(!read_file_to_buffer(vertex_path, (void**)&vertex_source, &vertex_source_size)) {
        ERROR("Failed to read vertex shader from file: %s", vertex_path);
        return program;
    }
    
    // Read fragment shader from file
    char* fragment_source = NULL;
    u64 fragment_source_size = 0;
    if(!read_file_to_buffer(fragment_path, (void**)&fragment_source, &fragment_source_size)) {
        ERROR("Failed to read fragment shader from file: %s", fragment_path);
        // Free vertex source memory before returning
        kfree(vertex_source, vertex_source_size, MEMORY_TAG_RENDERER);
        return program;
    }
    
    // Create the shader program
    program = shader_create_from_source(vertex_source, fragment_source);
    
    // Free the source code memory
    kfree(vertex_source, vertex_source_size, MEMORY_TAG_RENDERER);
    kfree(fragment_source, fragment_source_size, MEMORY_TAG_RENDERER);
    
    return program;
}

void shader_destroy(shader_program* program) {
    if (!program) return;
    
    // Detach shaders
    if (program->vertex_shader_id) {
        glDetachShader(program->program_id, program->vertex_shader_id);
    }
    
    if (program->fragment_shader_id) {
        glDetachShader(program->program_id, program->fragment_shader_id);
    }
    
    // Delete shaders
    if (program->vertex_shader_id) {
        glDeleteShader(program->vertex_shader_id);
    }
    
    if (program->fragment_shader_id) {
        glDeleteShader(program->fragment_shader_id);
    }
    
    // Delete program
    if (program->program_id) {
        glDeleteProgram(program->program_id);
    }
    
    // Zero out the program
    program->program_id = 0;
    program->vertex_shader_id = 0;
    program->fragment_shader_id = 0;
}

void shader_bind(const shader_program* program) {
    if (program && program->program_id) {
        glUseProgram(program->program_id);
    }
}

void shader_unbind(void) {
    glUseProgram(0);
}

void shader_set_mat4(const shader_program* program, const char* name, const void* value, b8 transpose) {
    if (!program || !program->program_id || !name || !value) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, transpose ? GL_TRUE : GL_FALSE, (const GLfloat*)value);
    }
}

void shader_set_vec4(const shader_program* program, const char* name, const void* value) {
    if (!program || !program->program_id || !name || !value) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniform4fv(location, 1, (const GLfloat*)value);
    }
}

void shader_set_vec3(const shader_program* program, const char* name, const void* value) {
    if (!program || !program->program_id || !name || !value) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniform3fv(location, 1, (const GLfloat*)value);
    }
}

void shader_set_vec2(const shader_program* program, const char* name, const void* value) {
    if (!program || !program->program_id || !name || !value) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniform2fv(location, 1, (const GLfloat*)value);
    }
}

void shader_set_int(const shader_program* program, const char* name, int value) {
    if (!program || !program->program_id || !name) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniform1i(location, value);
        // INFO("Set uniform '%s' to value %d in shader %u", name, value, program->program_id);
    } else {
        WARN("Uniform '%s' not found in shader %u", name, program->program_id);
    }
}

void shader_set_float(const shader_program* program, const char* name, float value) {
    if (!program || !program->program_id || !name) return;
    
    GLint location = glGetUniformLocation(program->program_id, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

// Internal helper functions

static void check_shader_error(u32 shader, const char* type) {
    i32 success;
    char info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        ERROR("Shader compilation error (%s): %s", type, info_log);
    }
}

static void check_program_error(u32 program) {
    i32 success;
    char info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, NULL, info_log);
        ERROR("Shader program linking error: %s", info_log);
    }
} 