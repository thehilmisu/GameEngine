#include "texture.h"
#include "core/logger.h"
#include "core/kstring.h"
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

static u32 next_texture_id = 0;

texture* texture_load(const char* file_path) {
    INFO("Loading texture from '%s'", file_path);
    
    texture* t = kallocate(sizeof(texture), MEMORY_TAG_TEXTURE);
    t->id = next_texture_id++;
    
    // Copy file path
    strncpy(t->path, file_path, sizeof(t->path) - 1);
    t->path[sizeof(t->path) - 1] = '\0'; // Ensure null termination
    
    // Load image data with stb_image
    stbi_set_flip_vertically_on_load(1); // Flip Y axis to match OpenGL's coordinate system
    int width, height, channels;
    unsigned char* data = stbi_load(file_path, &width, &height, &channels, 0);
    
    if (!data) {
        ERROR("Failed to load texture from '%s': %s", file_path, stbi_failure_reason());
        kfree(t, sizeof(texture), MEMORY_TAG_TEXTURE);
        return NULL;
    }
    
    // Store image info
    t->width = width;
    t->height = height;
    t->channels = channels;
    t->data = data;
    
    // Create OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data based on number of channels
    GLenum format;
    switch (channels) {
        case 1: format = GL_RED; break;
        case 2: format = GL_RG; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            ERROR("Unsupported number of channels: %d", channels);
            stbi_image_free(data);
            kfree(t, sizeof(texture), MEMORY_TAG_TEXTURE);
            return NULL;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Store OpenGL texture ID
    t->id = texture_id;
    
    // Free image data as it's now uploaded to GPU
    stbi_image_free(data);
    t->data = NULL;
    
    INFO("Texture loaded successfully: %dx%d, %d channels, ID: %u", width, height, channels, t->id);
    
    return t;
}

texture* texture_create(unsigned char* data, u32 width, u32 height, u32 channels) {
    texture* t = kallocate(sizeof(texture), MEMORY_TAG_TEXTURE);
    t->id = next_texture_id++;
    t->width = width;
    t->height = height;
    t->channels = channels;
    t->data = NULL;
    t->path[0] = '\0'; // Empty path for generated textures
    
    // Create OpenGL texture
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload texture data based on number of channels
    GLenum format;
    switch (channels) {
        case 1: format = GL_RED; break;
        case 2: format = GL_RG; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            ERROR("Unsupported number of channels: %d", channels);
            kfree(t, sizeof(texture), MEMORY_TAG_TEXTURE);
            return NULL;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Store OpenGL texture ID
    t->id = texture_id;
    
    INFO("Texture created successfully: %dx%d, %d channels, ID: %u", width, height, channels, t->id);
    
    return t;
}

void texture_destroy(texture* t) {
    if (!t) return;
    
    // Delete OpenGL texture
    glDeleteTextures(1, &t->id);
    
    // Free any remaining data
    if (t->data) {
        stbi_image_free(t->data);
        t->data = NULL;
    }
    
    // Free the texture struct
    kfree(t, sizeof(texture), MEMORY_TAG_TEXTURE);
}

void texture_bind(texture* t, u32 unit) {
    if (!t) return;
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, t->id);
}

void texture_unbind_all() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create a default checkerboard texture
texture* texture_create_default_checkerboard() {
    // Create a simple 8x8 checkerboard pattern (64 pixels)
    const u32 width = 8;
    const u32 height = 8;
    const u32 channels = 4; // RGBA
    
    // Allocate memory for the texture data (width * height * channels bytes)
    unsigned char* data = (unsigned char*)kallocate(width * height * channels, MEMORY_TAG_TEXTURE);
    
    // Create a black and white checkerboard pattern
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 offset = (y * width + x) * channels;
            
            // Check if this should be a white or black square
            u8 color = ((x + y) % 2 == 0) ? 255 : 0;
            
            // Set RGBA values
            data[offset + 0] = color; // R
            data[offset + 1] = color; // G
            data[offset + 2] = color; // B
            data[offset + 3] = 255;   // A (fully opaque)
        }
    }
    
    // Create the texture from the data
    texture* t = texture_create(data, width, height, channels);
    
    // Clean up the data as it's now copied to the GPU
    kfree(data, width * height * channels, MEMORY_TAG_TEXTURE);
    
    return t;
}
