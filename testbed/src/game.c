#include "game.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_frontend.h"
#include <stdio.h>
#include <math.h>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define MAX and MIN macros if not already defined
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// Event handler function
b8 game_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    game_state* state = (game_state*)listener_inst;
    if (!state) return FALSE;

    switch (code) {
        case EVENT_CODE_KEY_PRESSED: {
            u16 key_code = context.data.u16[0];
            if (key_code == ' ') {
                // Toggle rotation speed
                state->rotation_speed = state->rotation_speed == 2.0f ? 0.0f : 2.0f;
                INFO("Rotation speed toggled: %.1f", state->rotation_speed);
                return TRUE;
            } else if (key_code == 'a' || key_code == 'A') {
                // Slow down rotation
                state->rotation_speed = MAX(0.0f, state->rotation_speed - 0.5f);
                INFO("Rotation speed decreased: %.1f", state->rotation_speed);
                return TRUE;
            } else if (key_code == 'd' || key_code == 'D') {
                // Speed up rotation
                state->rotation_speed = MIN(4.0f, state->rotation_speed + 0.5f);
                INFO("Rotation speed increased: %.1f", state->rotation_speed);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

b8 game_initialize(game* game_instance) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = 0.0f;
    state->clear_color = (vec4){{0.0f, 0.0f, 0.2f, 1.0f}};
    state->rotation = 0.0f;
    state->rotation_speed = 2.0f;  // Initial rotation speed

    // Register for keyboard events
    event_register(EVENT_CODE_KEY_PRESSED, state, game_on_event);

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

    // Update rotation with variable speed
    state->rotation += delta_time * state->rotation_speed;
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

    // Unregister from keyboard events
    event_unregister(EVENT_CODE_KEY_PRESSED, state, game_on_event);

    if (state->triangle_mesh) {
        renderer_destroy_mesh(state->triangle_mesh);
        state->triangle_mesh = NULL;
    }
}
