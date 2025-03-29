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
// Create a cube - a more obvious 3D shape
static const vertex vertices[] = {
    // Front face (red)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}},  // bottom-right
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}},   // top-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}},   // top-right
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 0.0f, 1.0f}}},  // top-left

    // Back face (green)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}}, // bottom-left
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}},  // top-left
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}},   // top-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}}, // bottom-left
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}},   // top-right
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 0.0f, 1.0f}}},  // bottom-right

    // Top face (blue)
    // Triangle 1
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}}, // back-left
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}},  // front-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}},   // front-right
    // Triangle 2
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}},   // front-right
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 0.0f, 1.0f, 1.0f}}},  // back-right

    // Bottom face (yellow)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}},  // back-right
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}},   // front-right
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}}, // back-left
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}},   // front-right
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 1.0f, 0.0f, 1.0f}}},  // front-left

    // Right face (magenta)
    // Triangle 1
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}}, // bottom-back
    {.position = (vec3){{1.0f, 1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}},  // top-back
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}},   // top-front
    // Triangle 2
    {.position = (vec3){{1.0f, -1.0f, -1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}}, // bottom-back
    {.position = (vec3){{1.0f, 1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}},   // top-front
    {.position = (vec3){{1.0f, -1.0f, 1.0f}}, .color = (vec4){{1.0f, 0.0f, 1.0f, 1.0f}}},  // bottom-front

    // Left face (cyan)
    // Triangle 1
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}}, // bottom-back
    {.position = (vec3){{-1.0f, -1.0f, 1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}},  // bottom-front
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}},   // top-front
    // Triangle 2
    {.position = (vec3){{-1.0f, -1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}}, // bottom-back
    {.position = (vec3){{-1.0f, 1.0f, 1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}},   // top-front
    {.position = (vec3){{-1.0f, 1.0f, -1.0f}}, .color = (vec4){{0.0f, 1.0f, 1.0f, 1.0f}}}   // top-back
};

// Event handler function
b8 game_on_event(u16 code, void *sender, void *listener_inst, event_context context)
{
    game_state *state = (game_state *)listener_inst;
    if (!state)
        return FALSE;

    switch (code)
    {
    case EVENT_CODE_KEY_PRESSED:
    {
        u16 key_code = context.data.u16[0];
        if (key_code == 'a' || key_code == 'A')
        {
            // Move camera left
            state->camera_position.x -= CAMERA_SPEED;
            INFO("Camera moved left: %.2f", state->camera_position.x);
            return TRUE;
        }
        else if (key_code == 'd' || key_code == 'D')
        {
            // Move camera right
            state->camera_position.x += CAMERA_SPEED;
            INFO("Camera moved right: %.2f", state->camera_position.x);
            return TRUE;
        }
        else if (key_code == 'w' || key_code == 'W')
        {
            // Move camera forward
            state->camera_position.z += CAMERA_SPEED;
            INFO("Camera moved forward: %.2f", state->camera_position.z);
            return TRUE;
        }
        else if (key_code == 's' || key_code == 'S')
        {
            // Move camera backward
            state->camera_position.z -= CAMERA_SPEED;
            INFO("Camera moved backward: %.2f", state->camera_position.z);
            return TRUE;
        }
        else if (key_code == 'q' || key_code == 'Q')
        {
            // Move camera up
            state->camera_position.y += CAMERA_SPEED;
            INFO("Camera moved up: %.2f", state->camera_position.y);
            return TRUE;
        }
        else if (key_code == 'e' || key_code == 'E')
        {
            // Move camera down
            state->camera_position.y -= CAMERA_SPEED;
            INFO("Camera moved down: %.2f", state->camera_position.y);
            return TRUE;
        }
        break;
    }
    case EVENT_CODE_KEY_RELEASED:
    {
        u16 key_code = context.data.u16[0];
        INFO("Key released: %d", key_code);
        break;
    }
    case EVENT_CODE_MOUSE_MOVED:
    {
        u16 x = context.data.u16[0];
        u16 y = context.data.u16[1];

        // Save current mouse position for next frame
        state->last_mouse_x = x;
        state->last_mouse_y = y;
        return TRUE;
    }
    case EVENT_CODE_MOUSE_WHEEL:
    {
        // u16 x_delta = context.data.u16[0];
        // u16 y_delta = context.data.u16[1];
        // u16 z_delta = context.data.u16[2];
        // INFO("Mouse wheel: %d, %d, %d", x_delta, y_delta, z_delta);
        break;
    }
    case EVENT_CODE_BUTTON_PRESSED:
    {
        u16 button = context.data.u16[0];
        INFO("Mouse button pressed: %d", button);
        break;
    }
    case EVENT_CODE_BUTTON_RELEASED:
    {
        u16 button = context.data.u16[0];
        INFO("Mouse button released: %d", button);
        break;
    }
    }
    return FALSE;
}

void update_mesh_rotation(game_state *state, f32 delta_time, u32 mesh_id)
{
    // Update mesh rotation for a specific mesh ID
    u64 mesh_count = darray_length(state->mesh_commands);
    // INFO("update_mesh_rotation: Processing %llu meshes, looking for mesh_id %u", mesh_count, mesh_id);

    for (u64 i = 0; i < mesh_count; ++i)
    {
        mesh_command *cmd = &state->mesh_commands[i];
        if (cmd->mesh && cmd->mesh->id == mesh_id)
        {
            // INFO("Updating rotation for mesh %llu with id %u", i, mesh_id);
            // Update mesh rotation with proper delta time and reasonable speed
            float rotation_speed = 45.0f; // 45 degrees per second

            // Add rotation based on delta time for smooth motion
            cmd->rotation.x += rotation_speed * delta_time;
            cmd->rotation.y += rotation_speed * delta_time;
            cmd->rotation.z += rotation_speed * delta_time * 0.5f; // Slower Z rotation

            // Keep rotation within 0-360 degrees
            while (cmd->rotation.x >= 360.0f)
                cmd->rotation.x -= 360.0f;
            while (cmd->rotation.y >= 360.0f)
                cmd->rotation.y -= 360.0f;
            while (cmd->rotation.z >= 360.0f)
                cmd->rotation.z -= 360.0f;
        }
    }
}

b8 game_initialize(game *game_instance)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return FALSE;

    // Create a new file
    char *file_path = "test.txt";
    char *file_content = "Hello, World!";
    u64 file_size = strlen(file_content);
    write_buffer_to_file(file_path, file_content, file_size);

    // Read the file
    char *buffer;
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

    // Initialize mesh and text commands arrays
    state->mesh_commands = darray_create(mesh_command);
    state->text_commands = darray_create(text_command);

    // Register for keyboard and mouse events
    event_register(EVENT_CODE_KEY_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_KEY_RELEASED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_MOVED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_WHEEL, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_RELEASED, state, game_on_event);

    // TODO: Move this to a separate function
    //  Create mesh with 36 vertices (6 faces, 2 triangles per face, 3 vertices per triangle)
    mesh *cube_mesh = renderer_create_mesh(vertices, 36);
    if (!cube_mesh)
    {
        ERROR("Failed to create cube mesh!");
        return FALSE;
    }
    mesh *cube_mesh2 = renderer_create_mesh(vertices, 36);
    if (!cube_mesh2)
    {
        ERROR("Failed to create cube mesh!");
        return FALSE;
    }
    // Create and add cube mesh command to the array
    render_mesh(state, cube_mesh, 
                (vec3){{0.0f, 0.0f, 0.0f}},            // position at origin
                (vec3){{0.0f, 0.0f, 0.0f}},            // no initial rotation 
                (vec3){{1.0f, 1.0f, 1.0f}},            // normal size (1x)
                (vec4){{1.0f, 1.0f, 1.0f, 1.0f}});     // white color
                
    render_mesh(state, cube_mesh2, 
                (vec3){{-3.0f, 0.0f, -2.0f}},          // position offset (left and back)
                (vec3){{45.0f, 45.0f, 0.0f}},          // initial rotation of 45 degrees on X and Y
                (vec3){{0.5f, 2.0f, 0.5f}},            // stretched in Y axis, smaller in X and Z
                (vec4){{0.0f, 1.0f, 0.0f, 1.0f}});     // green color

    state->font = renderer_get_default_font();
    render_text(state, "Hello, World!", (vec2){{50.0f, 50.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);
    render_text(state, "Hello, World!", (vec2){{100.0f, 100.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);
    render_text(state, "Hello, World!", (vec2){{150.0f, 150.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    return TRUE;
}

b8 game_update(game *game_instance, f32 delta_time)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return FALSE;

    state->delta_time = delta_time;
    state->fps = 1.0f / delta_time;

    // Update all mesh rotations
    u64 mesh_count = darray_length(state->mesh_commands);
    for (u64 i = 0; i < mesh_count; ++i)
    {
        // Update the mesh with id 1
        if (state->mesh_commands[i].mesh->id == 1)
        {
            update_mesh_rotation(state, delta_time, state->mesh_commands[i].mesh->id);
        }
    }

    return TRUE;
}

b8 game_render(game *game_instance, f32 delta_time)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return FALSE;

    // Set camera position in the render packet
    render_packet current_packet;
    current_packet.camera_position = state->camera_position;
    current_packet.camera_rotation = state->camera_rotation;

    // Initialize command counts
    current_packet.text_commands.count = 0;

    // Set mesh commands from the darray
    u64 mesh_count = darray_length(state->mesh_commands);
    if (mesh_count > 0)
    {
        current_packet.mesh_commands.commands = state->mesh_commands;
        current_packet.mesh_commands.count = (u32)mesh_count;
    }

    // Set text commands from the darray
    u64 text_count = darray_length(state->text_commands);
    if (text_count > 0)
    {
        current_packet.text_commands.commands = state->text_commands;
        current_packet.text_commands.count = (u32)text_count;
    }

    // Draw the frame
    renderer_draw_frame(&current_packet);

    return TRUE;
}

void game_on_resize(game *game_instance, u32 width, u32 height)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return;
    // Handle window resize event
}

void game_shutdown(game *game_instance)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return;

    // Unregister from events
    event_unregister(EVENT_CODE_KEY_PRESSED, state, game_on_event);
    event_unregister(EVENT_CODE_KEY_RELEASED, state, game_on_event);
    event_unregister(EVENT_CODE_MOUSE_MOVED, state, game_on_event);
    event_unregister(EVENT_CODE_MOUSE_WHEEL, state, game_on_event);
    event_unregister(EVENT_CODE_BUTTON_PRESSED, state, game_on_event);
    event_unregister(EVENT_CODE_BUTTON_RELEASED, state, game_on_event);

    // Destroy meshes in the darray
    u64 mesh_count = darray_length(state->mesh_commands);
    for (u64 i = 0; i < mesh_count; ++i)
    {
        if (state->mesh_commands[i].mesh)
        {
            renderer_destroy_mesh(state->mesh_commands[i].mesh);
            state->mesh_commands[i].mesh = NULL;
        }
    }

    // Destroy the darray
    if (state->mesh_commands)
    {
        darray_destroy(state->mesh_commands);
        state->mesh_commands = NULL;
    }
    if (state->text_commands)
    {
        darray_destroy(state->text_commands);
        state->text_commands = NULL;
    }
}

void render_mesh(game_state *state, mesh *mesh, vec3 position, vec3 rotation, vec3 scale, vec4 color)
{
    mesh_command mesh_cmd = {
        .mesh = mesh,
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .color = color};
    darray_push(state->mesh_commands, mesh_cmd);
    INFO("Added mesh to render list. Position: (%.2f, %.2f, %.2f), Total count: %llu",
         position.x, position.y, position.z, darray_length(state->mesh_commands));
}

void render_text(game_state *state, const char *text, vec2 position, vec4 color, f32 scale, font *font)
{
    text_command text_cmd = {
        .text = text,
        .position = position,
        .color = color,
        .scale = scale,
        .font = font};
    darray_push(state->text_commands, text_cmd);
}