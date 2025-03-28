#include "game.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_frontend.h"
#include <stdio.h>
#include <math.h>


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
                // Move camera left
                state->camera_position.x -= 0.1f;
                INFO("Camera moved left: %.2f", state->camera_position.x);
                return TRUE;
            } else if (key_code == 'd' || key_code == 'D') {
                // Move camera right
                state->camera_position.x += 0.1f;
                INFO("Camera moved right: %.2f", state->camera_position.x);
                return TRUE;
            } else if (key_code == 'w' || key_code == 'W') {
                // Move camera forward
                state->camera_position.z -= 0.1f;
                INFO("Camera moved forward: %.2f", state->camera_position.z);
                return TRUE;
            } else if (key_code == 's' || key_code == 'S') {
                // Move camera backward
                state->camera_position.z += 0.1f;
                INFO("Camera moved backward: %.2f", state->camera_position.z);
                return TRUE;
            } else if (key_code == 'q' || key_code == 'Q') {
                // Move camera up
                state->camera_position.y += 0.1f;
                INFO("Camera moved up: %.2f", state->camera_position.y);
                return TRUE;
            } else if (key_code == 'e' || key_code == 'E') {
                // Move camera down
                state->camera_position.y -= 0.1f;
                INFO("Camera moved down: %.2f", state->camera_position.y);
                return TRUE;
            }
            break;
        }
        case EVENT_CODE_KEY_RELEASED: {
            u16 key_code = context.data.u16[0];
            INFO("Key released: %d", key_code);
            break;
        }
        case EVENT_CODE_MOUSE_MOVED: {
            // u16 x = context.data.u16[0];
            // u16 y = context.data.u16[1];
            // // Update camera rotation based on mouse movement
            // f32 sensitivity = 0.01f;
            // state->camera_rotation.y += (x - state->last_mouse_x) * sensitivity;
            // state->camera_rotation.x += (y - state->last_mouse_y) * sensitivity;
            // state->last_mouse_x = x;
            // state->last_mouse_y = y;
            // INFO("Camera rotation: %.2f, %.2f", state->camera_rotation.x, state->camera_rotation.y);
            break;
        }
        case EVENT_CODE_MOUSE_WHEEL: {
            u16 x_delta = context.data.u16[0];
            u16 y_delta = context.data.u16[1];
            u16 z_delta = context.data.u16[2];
            INFO("Mouse wheel: %d, %d, %d", x_delta, y_delta, z_delta);
            break;
        }   
        case EVENT_CODE_BUTTON_PRESSED: {
            u16 button = context.data.u16[0];
            INFO("Mouse button pressed: %d", button);
            break;
        }
        case EVENT_CODE_BUTTON_RELEASED: {  
            u16 button = context.data.u16[0];
            INFO("Mouse button released: %d", button);
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

    // Initialize camera
    state->camera_position = (vec3){{0.0f, 0.0f, 3.0f}};
    state->camera_rotation = (vec3){{0.0f, 0.0f, 0.0f}};
    state->last_mouse_x = 0;
    state->last_mouse_y = 0;

    // Load default font
    state->default_font = renderer_create_font("assets/fonts/NotoMono-Regular.ttf", 48);
    if (!state->default_font) {
        WARN("Failed to load primary font, trying fallback font...");
        state->default_font = renderer_create_fallback_font(48);
        if (!state->default_font) {
            ERROR("Failed to load fallback font!");
            return FALSE;
        }
        INFO("Fallback font loaded successfully");
    }

    // Register for keyboard and mouse events
    event_register(EVENT_CODE_KEY_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_KEY_RELEASED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_MOVED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_WHEEL, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_RELEASED, state, game_on_event);

    //TODO: Move this to a separate function
    // Triangle Vertices
    vertex vertices[] = {
        {
            .position = (vec3){{0.0f, 3.0f, 0.0f}},  // Top vertex - much higher
            .color = (vec4){{1.0f, 0.2f, 0.2f, 1.0f}}  // Brighter red
        },
        {
            .position = (vec3){{-3.0f, -3.0f, 0.0f}},  // Bottom left - much farther left and down
            .color = (vec4){{0.2f, 1.0f, 0.2f, 1.0f}}  // Brighter green
        },
        {
            .position = (vec3){{3.0f, -3.0f, 0.0f}},  // Bottom right - much farther right and down
            .color = (vec4){{0.2f, 0.2f, 1.0f, 1.0f}}  // Brighter blue
        }
    };

    // Create Triangle mesh 
    state->triangle_mesh = renderer_create_mesh(vertices, 3);
    if (!state->triangle_mesh) {
        ERROR("Failed to create triangle mesh!");
        return FALSE;
    }
    ///////////////////////////////////////////////////////////////////////

    return TRUE;
}

b8 game_update(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = delta_time;

    // Update rotation with variable speed
    state->rotation += delta_time * state->rotation_speed;
    // INFO("Rotation: %.2f", state->rotation);
    // if (state->rotation > 2.0f * M_PI) {
    //     state->rotation -= 2.0f * M_PI;
    // }

    return TRUE;
}

b8 game_render(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    // Set camera position in the render packet
    render_packet current_packet;  
    current_packet.camera_position = state->camera_position;
    
    // Set rotation value in camera_rotation.y for the shader
    state->camera_rotation.y = state->rotation;
    current_packet.camera_rotation = state->camera_rotation;
    
    // Set mesh rotation in render packet
    mesh_command mesh_cmd = {
        .position = {0.0f, 0.0f, 0.0f},
        .rotation = {0.0f, state->rotation, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f},
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .mesh = state->triangle_mesh
    };
    current_packet.mesh_commands.commands = &mesh_cmd;
    current_packet.mesh_commands.count = 1;
    
    text_command text_cmd1 = {
        .text = "Hello, World!",
        .position = (vec2){50.0f, 50.0f},  // Position near top-left corner
        .color = (vec4){1.0f, 1.0f, 1.0f, 1.0f},
        .scale = 1.0f,
        .font = state->default_font
    };
    current_packet.text_commands.commands = &text_cmd1;
    current_packet.text_commands.count = 1;
    
    if (state->triangle_mesh) {
        renderer_draw_frame(&current_packet);
    }
   
    return TRUE;
}

void game_on_resize(game* game_instance, u32 width, u32 height) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return;
    // Handle window resize event
}

void game_shutdown(game* game_instance) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return;
    // Unregister from events
    event_unregister(EVENT_CODE_KEY_PRESSED, state, game_on_event);
    event_unregister(EVENT_CODE_KEY_RELEASED, state, game_on_event);
    event_unregister(EVENT_CODE_MOUSE_MOVED, state, game_on_event);
    event_unregister(EVENT_CODE_MOUSE_WHEEL, state, game_on_event);
    event_unregister(EVENT_CODE_BUTTON_PRESSED, state, game_on_event);
    event_unregister(EVENT_CODE_BUTTON_RELEASED, state, game_on_event);

    if (state->triangle_mesh) {
        renderer_destroy_mesh(state->triangle_mesh);
        state->triangle_mesh = NULL;
    }

    if (state->default_font) {
        renderer_destroy_font(state->default_font);
        state->default_font = NULL;
    }
}
