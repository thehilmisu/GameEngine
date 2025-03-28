#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "opengl/opengl_renderer.h"
#include "core/logger.h"
#include "core/kmemory.h"
#include <stddef.h>  // For NULL

struct platform_state;

// Backend render context.
static renderer_backend* backend = 0;

b8 renderer_initialize(const char* application_name, struct platform_state* plat_state) {
    backend = kallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    // TODO: make this configurable.
    backend->initialize = opengl_renderer_backend_initialize;
    backend->shutdown = opengl_renderer_backend_shutdown;
    backend->begin_frame = opengl_renderer_backend_begin_frame;
    backend->end_frame = opengl_renderer_backend_end_frame;
    backend->resized = opengl_renderer_backend_resized;
    backend->create_mesh = opengl_renderer_create_mesh;
    backend->destroy_mesh = opengl_renderer_destroy_mesh;
    backend->draw_mesh = opengl_renderer_draw_mesh;

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
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    backend->frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    if (!backend) {
        ERROR("renderer_draw_frame called with no active renderer backend!");
        return FALSE;
    }

    // Begin frame
    if (!backend->begin_frame(backend, packet->delta_time)) {
        ERROR("renderer_begin_frame failed!");
        return FALSE;
    }

    // Update the current packet in the renderer state
    opengl_renderer_state* state = (opengl_renderer_state*)backend->internal_state;
    state->current_packet = packet;

    // Draw meshes
    if (packet && packet->meshes && packet->mesh_count > 0) {
        for (u32 i = 0; i < packet->mesh_count; ++i) {
            backend->draw_mesh(&packet->meshes[i]);
        }
    }

    // End frame
    b8 result = backend->end_frame(backend, packet->delta_time);
    if (!result) {
        ERROR("renderer_end_frame failed!");
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
        ERROR("renderer_create_mesh called with no active renderer backend!");
        return NULL;
    }
    return backend->create_mesh(vertices, vertex_count);
}

void renderer_destroy_mesh(mesh* m) {
    if (!backend) {
        ERROR("renderer_destroy_mesh called with no active renderer backend!");
        return;
    }
    backend->destroy_mesh(m);
}

void renderer_draw_mesh(mesh* m) {
    if (!backend) {
        ERROR("renderer_draw_mesh called with no active renderer backend!");
        return;
    }
    backend->draw_mesh(m);
}
