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
render_packet current_packet = {0};  // Make accessible to other files

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
    backend->destroy_font = opengl_renderer_destroy_font;
    backend->draw_text = opengl_renderer_draw_text;

    // Initialize the backend
    b8 result = backend->initialize(backend, application_name, plat_state);
    if (!result) {
        ERROR("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }

    return TRUE;
}

void renderer_shutdown() {
    if (backend) {
        backend->shutdown(backend);
        kfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
        backend = 0;
    }
}

b8 renderer_begin_frame(f32 delta_time) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return FALSE;
    }
    // Reset the current packet
    kzero_memory(&current_packet, sizeof(render_packet));
    current_packet.delta_time = delta_time;
    return backend->begin_frame(backend, &current_packet);
}

b8 renderer_end_frame(f32 delta_time) {
    if (!backend) {
        ERROR("Renderer backend not initialized!");
        return FALSE;
    }
    b8 result = backend->end_frame(backend, &current_packet);
    backend->frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    if (!packet) return FALSE;
    
    // Begin frame
    if (!backend->begin_frame(backend, packet)) {
        return FALSE;
    }
    
    // Draw text commands
    if (packet->text_commands.commands && packet->text_commands.count > 0) {
        for (u32 i = 0; i < packet->text_commands.count; i++) {
            backend->draw_text(backend->default_font, 
                             packet->text_commands.commands[i].text,
                             packet->text_commands.commands[i].position,
                             packet->text_commands.commands[i].color,
                             packet->text_commands.commands[i].scale);
        }
    }
    
    // Draw mesh commands
    if (packet->mesh_commands.commands && packet->mesh_commands.count > 0) {
        for (u32 i = 0; i < packet->mesh_commands.count; i++) {
            mesh* m = backend->get_mesh(packet->mesh_commands.commands[i].mesh_id);
            if (m) {
                backend->draw_mesh(m);
            }
        }
    }
    
    // End frame
    if (!backend->end_frame(backend, packet)) {
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
