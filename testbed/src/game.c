#include "game.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_frontend.h"
#include <stdio.h>
#include <math.h>

b8 game_initialize(game* game_instance) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = 0.0f;
    state->clear_color = (vec4){{0.0f, 0.0f, 0.2f, 1.0f}};
    state->rotation = 0.0f;

    // Create a triangle mesh
    vertex vertices[] = {
        {
            .position = (vec3){{0.0f, 0.5f, 0.0f}},
            .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}
        },
        {
            .position = (vec3){{-0.5f, -0.5f, 0.0f}},
            .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}
        },
        {
            .position = (vec3){{0.5f, -0.5f, 0.0f}},
            .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}
        }
    };

    state->triangle_mesh = renderer_create_mesh(vertices, 3);
    if (!state->triangle_mesh) {
        ERROR("Failed to create triangle mesh!");
        return FALSE;
    }

    return TRUE;
}

b8 game_update(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = delta_time;

    // Update rotation
    state->rotation += delta_time * 2.0f;
    if (state->rotation > 2.0f * M_PI) {
        state->rotation -= 2.0f * M_PI;
    }

    return TRUE;
}

b8 game_render(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    render_packet packet = {0};
    packet.meshes = NULL;
    packet.mesh_count = 0;
    packet.rotation = state->rotation;  // Pass rotation to renderer

    if (state->triangle_mesh) {
        packet.meshes = state->triangle_mesh;
        packet.mesh_count = 1;
    }

    return renderer_draw_frame(&packet);
}

void game_on_resize(game* game_instance, u32 width, u32 height) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return;

    // Handle window resize event
}

void game_shutdown(game* game_instance) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return;

    if (state->triangle_mesh) {
        renderer_destroy_mesh(state->triangle_mesh);
        state->triangle_mesh = NULL;
    }
}
