#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "opengl/opengl_renderer.h"
#include "core/logger.h"
#include "core/kmemory.h"
#include <stddef.h>  // For NULL
#include "platform/platform.h"

struct platform_state;

// Backend render context.
static renderer_backend* backend = 0;
static font* default_font = 0;

b8 renderer_initialize(const char* application_name, struct platform_state* plat_state) {
    backend = kallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
    backend->plat_state = plat_state;
    backend->frame_number = 0;

    // TODO: make this configurable.
    backend->initialize = opengl_renderer_backend_initialize;
    backend->shutdown = opengl_renderer_backend_shutdown;
    backend->begin_frame = opengl_renderer_backend_begin_frame;
    backend->end_frame = opengl_renderer_backend_end_frame;
    backend->resized = opengl_renderer_backend_resized;
    backend->create_mesh = opengl_renderer_create_mesh;
    backend->destroy_mesh = opengl_renderer_destroy_mesh;
    backend->draw_mesh = opengl_renderer_draw_mesh;
    backend->create_font = opengl_renderer_create_font;
    backend->create_fallback_font = opengl_renderer_create_fallback_font;
    backend->destroy_font = opengl_renderer_destroy_font;
    backend->draw_text = opengl_renderer_draw_text;

    // Initialize the backend
    b8 result = backend->initialize(backend, application_name, plat_state);
    if (!result) {
        ERROR("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }
     // Load default font
    default_font = renderer_create_font("assets/fonts/NotoMono-Regular.ttf", 48);
    if (!default_font) {
        WARN("Failed to load primary font, trying fallback font...");
        default_font = renderer_create_fallback_font(48);
        if (!default_font) {
            ERROR("Failed to load fallback font!");
            return FALSE;
        }
        INFO("Fallback font loaded successfully");
    }


    return TRUE;
}

font* renderer_get_default_font() {
    return default_font;
}

void renderer_shutdown() {
    if (backend) {
        backend->shutdown(backend);
        kfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
        backend = 0;
    }
    if (default_font) {
        renderer_destroy_font(default_font);
        default_font = NULL;
    }
}

b8 renderer_begin_frame(render_packet* packet, f32 delta_time) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return FALSE;
    }
    // Reset the current packet
    kzero_memory(&packet, sizeof(render_packet));
    packet->delta_time = delta_time;
    return backend->begin_frame(backend, packet);
}

b8 renderer_end_frame(render_packet* packet) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return FALSE;
    }
    b8 result = backend->end_frame(backend, packet);
    backend->frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    if (!packet) return FALSE;
    
    // Begin frame
    if (!backend->begin_frame(backend, packet)) {
        ERROR("Renderer backend failed to begin frame!");
        return FALSE;
    }
    
    // Draw text commands
    if (packet->text_commands.commands && packet->text_commands.count > 0) {
        for (u32 i = 0; i < packet->text_commands.count; i++) {
            // Use INFO for critical debugging info, but not all of it to avoid spam
            // INFO("Drawing text: %s", packet->text_commands.commands[i].text);                                                                           
            // INFO("Position: %f, %f", packet->text_commands.commands[i].position.x, packet->text_commands.commands[i].position.y);
            // INFO("Color: %f, %f, %f, %f", packet->text_commands.commands[i].color.x, packet->text_commands.commands[i].color.y, packet->text_commands.commands[i].color.z, packet->text_commands.commands[i].color.w);
            // INFO("Scale: %f", packet->text_commands.commands[i].scale);
            // INFO("Font pointer: %p", packet->text_commands.commands[i].font);
            
            if (packet->text_commands.commands[i].font) {
                backend->draw_text(packet->text_commands.commands[i].font, 
                                 packet->text_commands.commands[i].text,
                                 packet->text_commands.commands[i].position,
                                 packet->text_commands.commands[i].color,
                                 packet->text_commands.commands[i].scale);
            } else {
                ERROR("Attempted to draw text with NULL font");
            }
        }
    } 
    
    // Draw mesh commands
    if (packet->mesh_commands.commands && packet->mesh_commands.count > 0) {
        for (u32 i = 0; i < packet->mesh_commands.count; i++) {
            backend->draw_mesh(packet->mesh_commands.commands[i].mesh);
        }
    }
    
    // End frame
    if (!backend->end_frame(backend, packet)) {
        ERROR("Renderer backend failed to end frame!");
        return FALSE;
    }
    
    return TRUE;
}

void renderer_on_resized(u16 width, u16 height) {
    if (backend) {
        backend->resized(backend, width, height);
    } else {
        WARN("renderer_on_resized called with no active renderer backend!");
    }
}

// Mesh functions
mesh* renderer_create_mesh(const vertex* vertices, u32 vertex_count) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return NULL;
    }
    return backend->create_mesh(vertices, vertex_count);
}

void renderer_destroy_mesh(mesh* m) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return;
    }
    backend->destroy_mesh(m);
}

void renderer_draw_mesh(mesh* m) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return;
    }
    backend->draw_mesh(m);
}

font* renderer_create_font(const char* font_path, u32 font_size) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return NULL;
    }
    return backend->create_font(font_path, font_size);
}

font* renderer_create_fallback_font(u32 font_size) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return NULL;
    }
    return backend->create_fallback_font(font_size);
}

void renderer_destroy_font(font* f) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return;
    }
    backend->destroy_font(f);
}

void renderer_draw_text(font* f, const char* text, vec2 position, vec4 color, f32 scale) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return;
    }
    backend->draw_text(f, text, position, color, scale);
}
