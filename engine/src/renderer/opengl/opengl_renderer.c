#include "opengl_renderer.h"
#include "../renderer_types.inl"
#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"
#include "core/file_operations.h"
#include "platform/platform.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>

// Forward declare check functions
void check_shader_error(u32 shader, const char* type);
void check_program_error(u32 program);
void check_gl_error(const char* op);


static u32 next_mesh_id = 0;
static opengl_renderer_state* global_renderer_state = NULL;

// Paths to common system fonts for fallback
static const char* fallback_font_paths[] = {
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",  // Linux (Debian/Ubuntu)
    "/usr/share/fonts/TTF/DejaVuSans.ttf",                           // Linux (Arch/Manjaro)
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",                        // Linux (Fedora)
    "/Library/Fonts/Arial.ttf",                                      // macOS
    "C:\\Windows\\Fonts\\arial.ttf",                                 // Windows
    NULL
};

b8 opengl_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    opengl_renderer_state* state = kallocate(sizeof(opengl_renderer_state), MEMORY_TAG_RENDERER);
    backend->internal_state = state;
    state->current_packet = NULL;
    state->plat_state = plat_state;
    state->window = plat_state->window_handle;
    global_renderer_state = state;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        ERROR("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return FALSE;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(backend->plat_state->window_handle);
    if (!gl_context) {
        ERROR("OpenGL context could not be created! SDL_Error: %s", SDL_GetError());
        return FALSE;
    }
    state->gl_context = gl_context;

    // Initialize GLEW
    GLenum glew_init_result = glewInit();
    if (glew_init_result != GLEW_OK) {
        ERROR("GLEW could not initialize! Error: %s", glewGetErrorString(glew_init_result));
        return FALSE;
    }

    // Initialize FreeType
    FT_Error ft_error = FT_Init_FreeType(&state->ft_library);
    if (ft_error) {
        ERROR("FreeType could not initialize! Error: %d", ft_error);
        return FALSE;
    }

    // Load standard shaders using the new shader system
    shader_program standard_shader = shader_create_from_files(
        "assets/shaders/standard_vertex.vert", 
        "assets/shaders/standard_frag.frag"
    );
    
    if (standard_shader.program_id == 0) {
        ERROR("Failed to create standard shader program");
        return FALSE;
    }
    
    state->shader_program = standard_shader.program_id;

    // Create and bind VAO
    glGenVertexArrays(1, &state->vao);
    glBindVertexArray(state->vao);

    // Create and bind VBO
    glGenBuffers(1, &state->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Set up text rendering VAO/VBO
    glGenVertexArrays(1, &state->text_vao);
    glGenBuffers(1, &state->text_vbo);
    glBindVertexArray(state->text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
    // Allocate enough memory for 6 vertices (2 triangles = 1 quad)
    glBufferData(GL_ARRAY_BUFFER, sizeof(text_vertex) * 6, NULL, GL_DYNAMIC_DRAW);

    // Set up vertex attributes for the text vertex structure
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, position));
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, tex_coords));
    glEnableVertexAttribArray(1);

    // Color attribute (4 floats)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, color));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create text shader using the shader system
    shader_program text_shader = shader_create_from_files(
        "assets/shaders/text_vertex.vert", 
        "assets/shaders/text_frag.frag"
    );
    
    if (text_shader.program_id == 0) {
        ERROR("Failed to create text shader program");
        return FALSE;
    }
    
    state->text_shader_program = text_shader.program_id;
    
    // Get window size for proper aspect ratio
    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);
    float aspect_ratio = (float)width / (float)height;
    
    // Initialize projection matrix with 45 degree FOV and extended far plane
    create_perspective_matrix(&state->projection_matrix, 45.0f, aspect_ratio, 0.1f, 1000.0f);
    
    // Initialize view and model matrices to identity
    state->view_matrix = (mat4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    state->model_matrix = (mat4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    return TRUE;
}

void opengl_renderer_backend_shutdown(renderer_backend* backend) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    if (!state) return;

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &state->vao);
    glDeleteBuffers(1, &state->vbo);
    
    // Clean up shaders using our new shader system
    shader_program standard_shader = {0};
    standard_shader.program_id = state->shader_program;
    shader_destroy(&standard_shader);
    
    shader_program text_shader = {0};
    text_shader.program_id = state->text_shader_program;
    shader_destroy(&text_shader);

    // Clean up FreeType
    FT_Done_FreeType(state->ft_library);

    // Clean up SDL
    SDL_GL_DeleteContext(state->gl_context);

    // Free state
    kfree(state, sizeof(opengl_renderer_state), MEMORY_TAG_RENDERER);
    global_renderer_state = NULL;
}

void opengl_renderer_backend_resized(renderer_backend* backend, u16 width, u16 height) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    glViewport(0, 0, width, height);
    
    // Update projection matrix with new aspect ratio
    float aspect_ratio = (float)width / (float)height;
    create_perspective_matrix(&state->projection_matrix, 45.0f, aspect_ratio, 0.1f, 1000.0f);
}

b8 opengl_renderer_backend_begin_frame(renderer_backend* backend, render_packet* packet) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    state->current_packet = packet;

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing and blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Update view matrix with current camera data
    if (packet) {
        create_view_matrix(&state->view_matrix, packet->camera_position, packet->camera_rotation);
    }

    return TRUE;
}

b8 opengl_renderer_backend_end_frame(renderer_backend* backend, render_packet* packet) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    SDL_GL_SwapWindow(backend->plat_state->window_handle);
    return TRUE;
}

// Mesh functions
mesh* opengl_renderer_create_mesh(const vertex* vertices, u32 vertex_count) {
    mesh* m = kallocate(sizeof(mesh), MEMORY_TAG_RENDERER);
    m->vertex_count = vertex_count;
    m->vertex_buffer_size = sizeof(vertex) * vertex_count;
    m->vertices = kallocate(m->vertex_buffer_size, MEMORY_TAG_RENDERER);
    m->id = next_mesh_id++;
    kcopy_memory(m->vertices, vertices, m->vertex_buffer_size);

    // Create and bind VAO
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // Create and bind VBO
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_buffer_size, m->vertices, GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return m;
}

void opengl_renderer_destroy_mesh(mesh* m) {
    if (!m) return;

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &m->vao);
    glDeleteBuffers(1, &m->vbo);

    // Free vertex data
    kfree(m->vertices, m->vertex_buffer_size, MEMORY_TAG_RENDERER);
    kfree(m, sizeof(mesh), MEMORY_TAG_RENDERER);
}

void opengl_renderer_draw_mesh(mesh* m) {
    if (!m) {
        ERROR("Cannot draw NULL mesh");
        return;
    }
    
    opengl_renderer_state* state = global_renderer_state;
    if (!state) {
        ERROR("Cannot draw mesh, renderer state is NULL");
        return;
    }
    
    // Create a shader_program struct from the program ID
    shader_program program = {0};
    program.program_id = state->shader_program;
    
    // Use the shader using our new shader system
    shader_bind(&program);
    
    // Create view matrix based on camera position and rotation if we have a packet
    if (state->current_packet) {
        // Create and set the view matrix based on camera position and rotation
        create_view_matrix(&state->view_matrix, 
                         state->current_packet->camera_position,
                         state->current_packet->camera_rotation);
        
        // Set the view matrix uniform using our shader system
        shader_set_mat4(&program, "view", &state->view_matrix, FALSE);
        
        // Set the projection matrix uniform using our shader system
        shader_set_mat4(&program, "projection", &state->projection_matrix, FALSE);
        
        // Find the matching mesh command to get position, rotation and scale values
        for (u32 i = 0; i < state->current_packet->mesh_commands.count; i++) {
            if (state->current_packet->mesh_commands.commands[i].mesh == m) {
                // Create model matrix from mesh properties
                create_model_matrix(&state->model_matrix,
                                  state->current_packet->mesh_commands.commands[i].position,
                                  state->current_packet->mesh_commands.commands[i].rotation,
                                  state->current_packet->mesh_commands.commands[i].scale);
                
                // Set the model matrix uniform using our shader system
                shader_set_mat4(&program, "model", &state->model_matrix, FALSE);
                break;
            }
        }
    } else {
        // Set identity matrices for model, view and projection when no packet is available
        mat4 identity = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        
        shader_set_mat4(&program, "model", &identity, FALSE);
        shader_set_mat4(&program, "view", &identity, FALSE);
        shader_set_mat4(&program, "projection", &identity, FALSE);
    }
    
    // Bind VAO and draw
    glBindVertexArray(m->vao);
    glDrawArrays(GL_TRIANGLES, 0, m->vertex_count);
    
    // Unbind
    glBindVertexArray(0);
}

mesh* opengl_renderer_get_mesh(u32 mesh_id) {
    // This is a stub - in a full implementation, we would look up the mesh by ID
    // For now, we're just going to force direct mesh drawing
    return NULL;
}

// Font functions
font* opengl_renderer_create_font(const char* font_path, u32 font_size) {
    opengl_renderer_state* state = (opengl_renderer_state*)global_renderer_state;
    if (!state) return NULL;

    INFO("Creating font from '%s' with size %u", font_path, font_size);
    
    font* f = kallocate(sizeof(font), MEMORY_TAG_RENDERER);
    f->id = next_mesh_id++;
    
    // Zero out the character data to start with
    kzero_memory(f->characters, sizeof(font_character) * 128);

    // Load font face
    FT_Face face;
    FT_Error error = FT_New_Face(state->ft_library, font_path, 0, &face);
    if (error) {
        ERROR("Failed to load font face: %d", error);
        kfree(f, sizeof(font), MEMORY_TAG_RENDERER);
        return NULL;
    }

    // Set font size
    error = FT_Set_Pixel_Sizes(face, 0, font_size);
    if (error) {
        ERROR("Failed to set font size: %d", error);
        FT_Done_Face(face);
        kfree(f, sizeof(font), MEMORY_TAG_RENDERER);
        return NULL;
    }

    INFO("Font loaded successfully with %ld glyphs", face->num_glyphs);
    
    // Store the shader program ID
    f->shader_program = state->text_shader_program;

    // Create VAO and VBO for text rendering
    glGenVertexArrays(1, &f->vao);
    glGenBuffers(1, &f->vbo);
    glBindVertexArray(f->vao);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(text_vertex) * 6, NULL, GL_DYNAMIC_DRAW);

    // Set vertex attributes - fix the position to use 3 components
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, tex_coords));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)offsetof(text_vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Load ASCII printable characters (32-127)
    // Use FT_LOAD_RENDER to get bitmap data
    for (u8 c = 32; c < 128; c++) {
        // Load glyph with render flag
        error = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (error) {
            ERROR("Failed to load character '%c' (ASCII %d): %d", c, c, error);
            continue;
        }
        
        if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
            // For space or empty glyphs, just store metrics
            f->characters[c].width = 0;
            f->characters[c].height = 0;
            f->characters[c].bearing_x = face->glyph->bitmap_left;
            f->characters[c].bearing_y = face->glyph->bitmap_top;
            f->characters[c].advance = face->glyph->advance.x;
            f->characters[c].texture_id = 0;  // No texture
            
            if (c == ' ') {
                INFO("Space character metrics: advance=%d", f->characters[c].advance >> 6);
            }
            continue;
        }
        
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // Set texture parameters first
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Copy bitmap data and upload to texture
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Important for bitmap fonts!
        
        // Upload bitmap directly - FreeType stores it as a simple grayscale
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0, GL_RED, GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer);
        
        // Store character info
        f->characters[c].texture_id = texture;
        f->characters[c].width = face->glyph->bitmap.width;
        f->characters[c].height = face->glyph->bitmap.rows;
        f->characters[c].bearing_x = face->glyph->bitmap_left;
        f->characters[c].bearing_y = face->glyph->bitmap_top;
        f->characters[c].advance = face->glyph->advance.x;
       #if 0 
        if (c >= 'A' && c <= 'Z') {
            INFO("Character '%c' metrics: size=%dx%d, bearing=(%d,%d), advance=%d",
                c,
                f->characters[c].width,
                f->characters[c].height,
                f->characters[c].bearing_x,
                f->characters[c].bearing_y,
                f->characters[c].advance >> 6);
        }
        #endif
    }

    // Unbind textures and clean up
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Clean up FreeType face
    FT_Done_Face(face);
    
    INFO("Font creation complete: id=%u", f->id);
    return f;
}

// Try to create a font from a list of fallback paths
font* opengl_renderer_create_fallback_font(u32 font_size) {
    opengl_renderer_state* state = (opengl_renderer_state*)global_renderer_state;
    if (!state) return NULL;
    
    // Try each fallback font path
    for (int i = 0; fallback_font_paths[i] != NULL; i++) {
        const char* font_path = fallback_font_paths[i];
        INFO("Trying fallback font: %s", font_path);
        
        // Check if file exists before attempting to load
        FILE* file = fopen(font_path, "r");
        if (file) {
            fclose(file);
            
            // Try to create the font
            font* f = opengl_renderer_create_font(font_path, font_size);
            if (f) {
                INFO("Successfully loaded fallback font: %s", font_path);
                return f;
            }
        }
    }
    
    ERROR("Failed to load any fallback fonts");
    return NULL;
}

b8 opengl_renderer_draw_frame(renderer_backend* backend, render_packet* packet) {
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    if (!state) return FALSE;

    // Check if frame has already started
    check_gl_error("before drawing frame");

    // Draw meshes first
    if (packet->mesh_commands.commands && packet->mesh_commands.count > 0) {
        // Enable depth testing for 3D objects
        glEnable(GL_DEPTH_TEST);
        for (u32 i = 0; i < packet->mesh_commands.count; i++) {
            opengl_renderer_draw_mesh(packet->mesh_commands.commands[i].mesh);
        }
    }
    
    // Draw text commands last, so they appear on top
    if (packet->text_commands.commands && packet->text_commands.count > 0) {
        // Disable depth testing for UI elements
        glDisable(GL_DEPTH_TEST);
        
        // INFO("Processing %u text commands", packet->text_commands.count);
        for (u32 i = 0; i < packet->text_commands.count; i++) {
            // INFO("Drawing text '%s' at (%.2f,%.2f)", 
            //      packet->text_commands.commands[i].text,
            //      packet->text_commands.commands[i].position.x,
            //      packet->text_commands.commands[i].position.y);
                 
            opengl_renderer_draw_text(
                packet->text_commands.commands[i].font,
                packet->text_commands.commands[i].text,
                packet->text_commands.commands[i].position,
                packet->text_commands.commands[i].color,
                packet->text_commands.commands[i].scale
            );
        }
        
        // Restore depth testing
        glEnable(GL_DEPTH_TEST);
    }
    
    check_gl_error("after drawing frame");
    return TRUE;
}

void opengl_renderer_destroy_font(font* f) {
    if (!f) return;

    // Delete character textures
    for (u8 c = 0; c < 128; c++) {
        if (f->characters[c].texture_id) {
            glDeleteTextures(1, &f->characters[c].texture_id);
        }
    }

    // Delete OpenGL resources
    glDeleteVertexArrays(1, &f->vao);
    glDeleteBuffers(1, &f->vbo);
    
    // Clean up the shader program using our shader system
    shader_program text_shader = {0};
    text_shader.program_id = f->shader_program;
    shader_destroy(&text_shader);

    // Free font data
    kfree(f, sizeof(font), MEMORY_TAG_RENDERER);
}

// Helper function to check for OpenGL errors
void check_gl_error(const char* op) {
    GLenum error;
    while((error = glGetError()) != GL_NO_ERROR) {
        const char* error_str = "Unknown";
        switch(error) {
            case GL_INVALID_ENUM: error_str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: error_str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: error_str = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW: error_str = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW: error_str = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY: error_str = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        ERROR("OpenGL error after %s: %s (0x%x)", op, error_str, error);
    }
}

void opengl_renderer_draw_text(font* f, const char* text, vec2 position, vec4 color, f32 scale) {
    if (!f || !text) {
        return;
    }
    
    opengl_renderer_state* state = global_renderer_state;
    if (!state) {
        ERROR("Cannot draw text, renderer state is NULL");
        return;
    }

    // Clear settings that could interfere
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Force blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create a shader_program struct from the program ID
    shader_program program = {0};
    program.program_id = f->shader_program;
    
    // Use the shader using our new shader system
    shader_bind(&program);
    
    // Get window size for proper projection matrix
    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);
    
    // Create orthographic projection matrix (screen space)
    mat4 projection = {
        2.0f/width, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f/height, 0.0f, 0.0f, 
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    // Set projection matrix uniform using our shader system
    shader_set_mat4(&program, "projection", &projection, FALSE);
    
    // Set texture unit using our shader system
    shader_set_int(&program, "textTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    
    // Bind font's VAO
    glBindVertexArray(f->vao);
    
    // Starting position for rendering
    f32 x = position.x;
    f32 y = position.y;
    
    // For each character in the string
    for (const char* c = text; *c; c++) {
        // Get character index
        u8 char_index = (u8)*c;
        if (char_index >= 128) continue; // Skip non-ASCII
        
        // Get character info
        font_character* ch = &f->characters[char_index];
        if (ch->texture_id == 0) {
            x += (ch->advance >> 6) * scale; // Advance cursor
            continue; // Skip characters with no texture (e.g., spaces)
        }
        
        // Calculate position and size
        f32 xpos = x + ch->bearing_x * scale;
        f32 ypos = y - ch->bearing_y * scale;  // Position based on the baseline
        f32 w = ch->width * scale;
        f32 h = ch->height * scale;
        
        // Skip if zero size
        if (w <= 0 || h <= 0) {
            x += (ch->advance >> 6) * scale; // Advance cursor
            continue;
        }
        
        // Define vertices for this character with corrected texture coordinates
        // Note: Texture coordinates are flipped vertically to correct upside-down rendering
        text_vertex vertices[6] = {
            // First triangle - top-left, bottom-left, bottom-right
            { {xpos,     ypos + h, 0.0f}, {0.0f, 1.0f}, color }, // top-left
            { {xpos,     ypos,     0.0f}, {0.0f, 0.0f}, color }, // bottom-left
            { {xpos + w, ypos,     0.0f}, {1.0f, 0.0f}, color }, // bottom-right
            
            // Second triangle - top-left, bottom-right, top-right
            { {xpos,     ypos + h, 0.0f}, {0.0f, 1.0f}, color }, // top-left
            { {xpos + w, ypos,     0.0f}, {1.0f, 0.0f}, color }, // bottom-right
            { {xpos + w, ypos + h, 0.0f}, {1.0f, 1.0f}, color }  // top-right
        };
        
        // Bind character texture
        glBindTexture(GL_TEXTURE_2D, ch->texture_id);
        
        // Update buffer
        glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Draw the character quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor for next character
        x += (ch->advance >> 6) * scale;
    }
    
    // Clean up
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// Shader error checking functions
void check_shader_error(u32 shader, const char* type) {
    i32 success;
    char info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        ERROR("Shader compilation error (%s): %s", type, info_log);
    }
}

void check_program_error(u32 program) {
    i32 success;
    char info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, NULL, info_log);
        ERROR("Shader program linking error: %s", info_log);
    }
}

// Matrix utility functions
void create_model_matrix(mat4* matrix, vec3 position, vec3 rotation, vec3 scale) {
    // Start with identity matrix
    *matrix = mat4_identity();
    
    // Convert rotation to radians
    float rotX = rotation.x * 3.14159f / 180.0f;
    float rotY = rotation.y * 3.14159f / 180.0f;
    float rotZ = rotation.z * 3.14159f / 180.0f;
    
    // Pre-calculate sines and cosines
    float cosX = cosf(rotX);
    float sinX = sinf(rotX);
    float cosY = cosf(rotY);
    float sinY = sinf(rotY);
    float cosZ = cosf(rotZ);
    float sinZ = sinf(rotZ);
    
    // X-axis rotation matrix
    mat4 rx = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosX, -sinX, 0.0f,
        0.0f, sinX, cosX, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Y-axis rotation matrix
    mat4 ry = {
        cosY, 0.0f, sinY, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinY, 0.0f, cosY, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Z-axis rotation matrix
    mat4 rz = {
        cosZ, sinZ, 0.0f, 0.0f,
        -sinZ, cosZ, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Scale matrix
    mat4 s = {
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Translation matrix
    mat4 t = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    
    // Combined matrix: T * Rz * Ry * Rx * S
    // Apply transformations in sequence using existing mat4_mul
    mat4 result = *matrix;
    result = mat4_mul(result, s);     // Apply scale
    result = mat4_mul(result, rx);    // Apply X rotation
    result = mat4_mul(result, ry);    // Apply Y rotation
    result = mat4_mul(result, rz);    // Apply Z rotation
    result = mat4_mul(result, t);     // Apply translation
    
    *matrix = result;
}

void create_view_matrix(mat4* matrix, vec3 camera_pos, vec3 camera_rotation) {
    // Start with identity matrix
    *matrix = mat4_identity();
    
    // Convert rotation to radians
    float rotX = camera_rotation.x * 3.14159f / 180.0f;
    float rotY = camera_rotation.y * 3.14159f / 180.0f;
    float rotZ = camera_rotation.z * 3.14159f / 180.0f;
    
    // Pre-calculate sines and cosines
    float cosX = cosf(rotX);
    float sinX = sinf(rotX);
    float cosY = cosf(rotY);
    float sinY = sinf(rotY);
    float cosZ = cosf(rotZ);
    float sinZ = sinf(rotZ);
    
    // Camera rotation matrices - first create rotation matrices
    // Order matters: Y (yaw) rotation first, then X (pitch), then Z (roll)
    
    // Y-axis rotation matrix (yaw - look left/right)
    mat4 ry = {
        cosY, 0.0f, -sinY, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sinY, 0.0f, cosY, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // X-axis rotation matrix (pitch - look up/down)
    mat4 rx = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosX, sinX, 0.0f,
        0.0f, -sinX, cosX, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Z-axis rotation matrix (roll - tilt head)
    mat4 rz = {
        cosZ, sinZ, 0.0f, 0.0f,
        -sinZ, cosZ, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    // Translation matrix (move to camera position)
    mat4 t = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -camera_pos.x, -camera_pos.y, -camera_pos.z, 1.0f
    };
    
    // Combined rotation matrix (apply rotations in YXZ order)
    mat4 rotation = mat4_identity();
    rotation = mat4_mul(rotation, ry);   // First rotate around Y
    rotation = mat4_mul(rotation, rx);   // Then rotate around X
    rotation = mat4_mul(rotation, rz);   // Finally rotate around Z
    
    // Final view matrix: first translate, then rotate
    *matrix = mat4_mul(rotation, t);
}

void create_perspective_matrix(mat4* matrix, float fov_degrees, float aspect_ratio, float near_plane, float far_plane) {
    // Convert FOV from degrees to radians
    float fov_radians = fov_degrees * 3.14159f / 180.0f;
    
    // Calculate perspective matrix elements
    float f = 1.0f / tanf(fov_radians * 0.5f);
    float nf = 1.0f / (near_plane - far_plane);
    
    // Create the perspective projection matrix
    *matrix = (mat4){
        f / aspect_ratio, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (far_plane + near_plane) * nf, -1.0f,
        0.0f, 0.0f, 2.0f * far_plane * near_plane * nf, 0.0f
    };
    
    // Debug output 
    INFO("Created perspective matrix with FOV=%.1f°, aspect=%.2f, near=%.2f, far=%.2f",
         fov_degrees, aspect_ratio, near_plane, far_plane);
} 