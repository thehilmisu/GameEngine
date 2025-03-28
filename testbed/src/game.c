#include "game.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_frontend.h"
#include "core/file_operations.h"
#include <stdio.h>
#include <math.h>

#define CAMERA_SPEED 0.5f
#define CAMERA_ROTATION_SPEED 1.5f 

// Event handler function
b8 game_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    game_state* state = (game_state*)listener_inst;
    if (!state) return FALSE;

    switch (code) {
        case EVENT_CODE_KEY_PRESSED: {
            u16 key_code = context.data.u16[0];
            if (key_code == 'a' || key_code == 'A') {
                // Move camera left
                state->camera_position.x -= CAMERA_SPEED;
                INFO("Camera moved left: %.2f", state->camera_position.x);
                return TRUE;
            } else if (key_code == 'd' || key_code == 'D') {
                // Move camera right
                state->camera_position.x += CAMERA_SPEED;
                INFO("Camera moved right: %.2f", state->camera_position.x);
                return TRUE;
            } else if (key_code == 'w' || key_code == 'W') {
                // Move camera forward
                state->camera_position.z += CAMERA_SPEED;
                INFO("Camera moved forward: %.2f", state->camera_position.z);
                return TRUE;
            } else if (key_code == 's' || key_code == 'S') {
                // Move camera backward
                state->camera_position.z -= CAMERA_SPEED;
                INFO("Camera moved backward: %.2f", state->camera_position.z);
                return TRUE;
            } else if (key_code == 'q' || key_code == 'Q') {
                // Move camera up
                state->camera_position.y += CAMERA_SPEED;
                INFO("Camera moved up: %.2f", state->camera_position.y);
                return TRUE;
            } else if (key_code == 'e' || key_code == 'E') {
                // Move camera down
                state->camera_position.y -= CAMERA_SPEED;
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
            u16 x = context.data.u16[0];
            u16 y = context.data.u16[1];
            
            // Save current mouse position for next frame
            state->last_mouse_x = x;
            state->last_mouse_y = y;
            return TRUE;
        }
        case EVENT_CODE_MOUSE_WHEEL: {
            // u16 x_delta = context.data.u16[0];
            // u16 y_delta = context.data.u16[1];
            // u16 z_delta = context.data.u16[2];
            // INFO("Mouse wheel: %d, %d, %d", x_delta, y_delta, z_delta);
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

    // Create a new file
    char* file_path = "test.txt";
    char* file_content = "Hello, World!";
    u64 file_size = strlen(file_content);
    write_buffer_to_file(file_path, file_content, file_size);

    // Read the file
    char* buffer;
    u64 size;
    read_file_to_buffer(file_path, &buffer, &size);
    INFO("File content: %s", buffer);
    free(buffer);

    state->delta_time = 0.0f;
    state->clear_color = (vec4){{0.0f, 0.0f, 0.2f, 1.0f}};
    state->fps = 0.0f;

    // Initialize camera with a better position to see 3D effects
    state->camera_position = (vec3){{2.5f, 2.5f, 3.5f}};
    state->camera_rotation = (vec3){{0.5f, 0.5f, 0.0f}};
    state->last_mouse_x = 0;
    state->last_mouse_y = 0;
    
    // Initialize mesh command with default values
    state->triangle_mesh_command.position = (vec3){{0.0f, 0.0f, 0.0f}};
    state->triangle_mesh_command.rotation = (vec3){{0.0f, 0.0f, 0.0f}};
    state->triangle_mesh_command.scale = (vec3){{1.0f, 1.0f, 1.0f}};
    state->triangle_mesh_command.color = (vec4){{1.0f, 1.0f, 1.0f, 1.0f}};

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
    // Create a cube - a more obvious 3D shape
    vertex vertices[] = {
        // Front face (red)
        // Triangle 1
        { .position = (vec3){{-1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // bottom-left
        { .position = (vec3){{ 1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // bottom-right
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // top-right
        // Triangle 2
        { .position = (vec3){{-1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // bottom-left
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // top-right
        { .position = (vec3){{-1.0f,  1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}} }, // top-left
        
        // Back face (green)
        // Triangle 1
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // bottom-left
        { .position = (vec3){{-1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // top-left
        { .position = (vec3){{ 1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // top-right
        // Triangle 2
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // bottom-left
        { .position = (vec3){{ 1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // top-right
        { .position = (vec3){{ 1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}} }, // bottom-right
        
        // Top face (blue)
        // Triangle 1
        { .position = (vec3){{-1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // back-left
        { .position = (vec3){{-1.0f,  1.0f,  1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // front-left
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // front-right
        // Triangle 2
        { .position = (vec3){{-1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // back-left
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // front-right
        { .position = (vec3){{ 1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}} }, // back-right
        
        // Bottom face (yellow)
        // Triangle 1
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // back-left
        { .position = (vec3){{ 1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // back-right
        { .position = (vec3){{ 1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // front-right
        // Triangle 2
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // back-left
        { .position = (vec3){{ 1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // front-right
        { .position = (vec3){{-1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}} }, // front-left
        
        // Right face (magenta)
        // Triangle 1
        { .position = (vec3){{ 1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // bottom-back
        { .position = (vec3){{ 1.0f,  1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // top-back
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // top-front
        // Triangle 2
        { .position = (vec3){{ 1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // bottom-back
        { .position = (vec3){{ 1.0f,  1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // top-front
        { .position = (vec3){{ 1.0f, -1.0f,  1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}} }, // bottom-front
        
        // Left face (cyan)
        // Triangle 1
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }, // bottom-back
        { .position = (vec3){{-1.0f, -1.0f,  1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }, // bottom-front
        { .position = (vec3){{-1.0f,  1.0f,  1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }, // top-front
        // Triangle 2
        { .position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }, // bottom-back
        { .position = (vec3){{-1.0f,  1.0f,  1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }, // top-front
        { .position = (vec3){{-1.0f,  1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}} }  // top-back
    };

    // Create mesh with 36 vertices (6 faces, 2 triangles per face, 3 vertices per triangle)
    state->triangle_mesh_command.mesh = renderer_create_mesh(vertices, 36);
    if (!state->triangle_mesh_command.mesh) {
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
    state->fps = 1.0f / delta_time;


    // Update mesh rotation with proper delta time and reasonable speed
    float rotation_speed = 45.0f; // 45 degrees per second
    
    // Add rotation based on delta time for smooth motion
    state->triangle_mesh_command.rotation.x += rotation_speed * delta_time;
    state->triangle_mesh_command.rotation.y += rotation_speed * delta_time;
    state->triangle_mesh_command.rotation.z += rotation_speed * delta_time * 0.5f; // Slower Z rotation
    
    // Keep rotation within 0-360 degrees
    while (state->triangle_mesh_command.rotation.x >= 360.0f) state->triangle_mesh_command.rotation.x -= 360.0f;
    while (state->triangle_mesh_command.rotation.y >= 360.0f) state->triangle_mesh_command.rotation.y -= 360.0f;
    while (state->triangle_mesh_command.rotation.z >= 360.0f) state->triangle_mesh_command.rotation.z -= 360.0f;
    
    // INFO("Mesh rotation: %f, %f, %f", state->triangle_mesh_command.rotation.x, state->triangle_mesh_command.rotation.y, state->triangle_mesh_command.rotation.z);
    
    return TRUE;
}

b8 game_render(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    // Set camera position in the render packet
    render_packet current_packet;  
    current_packet.camera_position = state->camera_position;
    current_packet.camera_rotation = state->camera_rotation;
    
    // Initialize command counts
    current_packet.text_commands.count = 0;
    
    // Set mesh command
    mesh_command mesh_cmd = {
        .position = state->triangle_mesh_command.position,
        .rotation = state->triangle_mesh_command.rotation,
        .scale = state->triangle_mesh_command.scale,
        .color = state->triangle_mesh_command.color,
        .mesh = state->triangle_mesh_command.mesh
    };
    current_packet.mesh_commands.commands = &mesh_cmd;
    current_packet.mesh_commands.count = 1;
    
    // Display FPS
    char fps_text[10];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", state->fps);
    
    if (state->triangle_mesh_command.mesh) {
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

    if (state->triangle_mesh_command.mesh) {
        renderer_destroy_mesh(state->triangle_mesh_command.mesh);
        state->triangle_mesh_command.mesh = NULL;
    }

    if (state->default_font) {
        renderer_destroy_font(state->default_font);
        state->default_font = NULL;
    }
}

void draw_text(game_state* state, const char* text, 
              vec2 position, vec4 color, f32 scale, font* font, 
              render_packet* packet) {
    text_command text_cmd = {
        .text = text,
        .position = position,
        .color = color,
        .scale = scale,
        .font = font
    };
    packet->text_commands.commands = &text_cmd;
    packet->text_commands.count += 1;    
}