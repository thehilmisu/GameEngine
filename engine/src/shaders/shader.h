#pragma once

#include "definitions.h"

typedef struct shader_program {
    u32 program_id;
    u32 vertex_shader_id;
    u32 fragment_shader_id;
} shader_program;

/**
 * @brief Creates a shader program from source code strings
 * 
 * @param vertex_source The vertex shader source code
 * @param fragment_source The fragment shader source code
 * @return shader_program The created shader program (program_id will be 0 if failed)
 */
shader_program shader_create_from_source(const char* vertex_source, const char* fragment_source);

/**
 * @brief Creates a shader program from shader files
 * 
 * @param vertex_path Path to the vertex shader file
 * @param fragment_path Path to the fragment shader file
 * @return shader_program The created shader program (program_id will be 0 if failed)
 */
shader_program shader_create_from_files(const char* vertex_path, const char* fragment_path);

/**
 * @brief Destroys a shader program and releases resources
 * 
 * @param program The shader program to destroy
 */
void shader_destroy(shader_program* program);

/**
 * @brief Binds (activates) a shader program for use
 * 
 * @param program The shader program to bind
 */
void shader_bind(const shader_program* program);

/**
 * @brief Unbinds the currently active shader program
 */
void shader_unbind(void);

/**
 * @brief Sets a uniform mat4 value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value Pointer to the mat4 value
 * @param transpose Whether to transpose the matrix
 */
void shader_set_mat4(const shader_program* program, const char* name, const void* value, b8 transpose);

/**
 * @brief Sets a uniform vec4 value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value Pointer to the vec4 value
 */
void shader_set_vec4(const shader_program* program, const char* name, const void* value);

/**
 * @brief Sets a uniform vec3 value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value Pointer to the vec3 value
 */
void shader_set_vec3(const shader_program* program, const char* name, const void* value);

/**
 * @brief Sets a uniform vec2 value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value Pointer to the vec2 value
 */
void shader_set_vec2(const shader_program* program, const char* name, const void* value);

/**
 * @brief Sets a uniform int value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value The int value
 */
void shader_set_int(const shader_program* program, const char* name, int value);

/**
 * @brief Sets a uniform float value in the shader
 * 
 * @param program The shader program to set the uniform in
 * @param name The name of the uniform
 * @param value The float value
 */
void shader_set_float(const shader_program* program, const char* name, float value); 