#include "renderer_backend.h"
#include "renderer/opengl/opengl_renderer.h"
#include "core/logger.h"

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* plat_state, renderer_backend* out_renderer_backend) {
    out_renderer_backend->plat_state = plat_state;

    switch (type) {
        case RENDERER_BACKEND_TYPE_OPENGL:
            out_renderer_backend->initialize = opengl_renderer_backend_initialize;
            out_renderer_backend->shutdown = opengl_renderer_backend_shutdown;
            out_renderer_backend->resized = opengl_renderer_backend_resized;
            out_renderer_backend->begin_frame = opengl_renderer_backend_begin_frame;
            out_renderer_backend->end_frame = opengl_renderer_backend_end_frame;
            out_renderer_backend->draw_frame = opengl_renderer_draw_frame;
            out_renderer_backend->create_mesh = opengl_renderer_create_mesh;
            out_renderer_backend->destroy_mesh = opengl_renderer_destroy_mesh;
            out_renderer_backend->draw_mesh = opengl_renderer_draw_mesh;
            out_renderer_backend->get_mesh = opengl_renderer_get_mesh;
            out_renderer_backend->create_model = opengl_renderer_create_model;
            out_renderer_backend->destroy_model = opengl_renderer_destroy_model;
            out_renderer_backend->draw_model = opengl_renderer_draw_model;
            out_renderer_backend->create_font = opengl_renderer_create_font;
            out_renderer_backend->destroy_font = opengl_renderer_destroy_font;
            out_renderer_backend->draw_text = opengl_renderer_draw_text;
            break;
        default:
            ERROR("Unsupported renderer backend type: %d", type);
            return FALSE;
    }

    return TRUE;
}

void renderer_backend_destroy(renderer_backend* renderer_backend) {
    renderer_backend->shutdown(renderer_backend);
}
