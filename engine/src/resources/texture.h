#pragma once

#include "renderer/renderer_types.inl"
#include "core/kmemory.h"

/**
 * @brief Loads a texture from a file path
 * 
 * @param file_path Path to the texture file
 * @return texture* Pointer to the loaded texture, or NULL if loading failed
 */
texture* texture_load(const char* file_path);

/**
 * @brief Creates a texture from raw data
 * 
 * @param data Raw pixel data
 * @param width Width of the texture
 * @param height Height of the texture
 * @param channels Number of channels (1=R, 2=RG, 3=RGB, 4=RGBA)
 * @return texture* Pointer to the created texture, or NULL if creation failed
 */
texture* texture_create(unsigned char* data, u32 width, u32 height, u32 channels);

/**
 * @brief Destroys a texture and frees its resources
 * 
 * @param texture The texture to destroy
 */
void texture_destroy(texture* texture);

/**
 * @brief Binds a texture to the given texture unit
 * 
 * @param texture The texture to bind
 * @param unit The texture unit to bind to (0-15)
 */
void texture_bind(texture* texture, u32 unit);

/**
 * @brief Unbinds all textures
 */
void texture_unbind_all();

/**
 * @brief Creates a default checkerboard texture
 * 
 * @return texture* Pointer to the created texture
 */
texture* texture_create_default_checkerboard();
