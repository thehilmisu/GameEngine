#pragma once

#include "definitions.h"
#include "renderer/renderer_types.inl"
#include "core/kmemory.h"

/**
 * @brief Loads a model from an OBJ file
 * 
 * @param file_path Path to the OBJ file
 * @return model* Pointer to the loaded model, or NULL if loading failed
 * 
 * @example
 * // Load a model from an OBJ file
 * model* cube = model_load_obj("bin/assets/models/cube.obj");
 * if (!cube) {
 *     ERROR("Failed to load cube model");
 *     return FALSE;
 * }
 * 
 * // Use the model in your game
 * vec3 position = {0.0f, 0.0f, -5.0f};
 * vec3 rotation = {0.0f, 45.0f, 0.0f};
 * vec3 scale = {1.0f, 1.0f, 1.0f};
 * vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
 * 
 * // Add to render list
 * mesh_command cmd = {
 *     .mesh = cube->mesh,
 *     .position = position,
 *     .rotation = rotation,
 *     .scale = scale,
 *     .color = color
 * };
 * darray_push(state->mesh_commands, cmd);
 * 
 * // Don't forget to destroy the model when done
 * model_destroy(cube);
 */
model* model_load_obj(const char* file_path);

/**
 * @brief Destroys a model and frees its resources
 * 
 * @param m The model to destroy
 */
void model_destroy(model* m);

/**
 * @brief Renders a model using the current renderer
 * 
 * @param m The model to render
 * @param position The position to render the model at
 * @param rotation The rotation of the model
 * @param scale The scale of the model
 * 
 * @note This function doesn't actually render the model immediately.
 * You must add the model's mesh to the render packet in your game code.
 * See the example in model_load_obj.
 */
void model_render(model* m, vec3 position, vec3 rotation, vec3 scale);

/**
 * @brief Gets a model by ID
 * 
 * @param model_id The ID of the model to get
 * @return model* Pointer to the model, or NULL if not found
 */
model* model_get_by_id(u32 model_id); 