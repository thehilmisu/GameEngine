#include "game.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_frontend.h"
#include "core/file_operations.h"
#include <stdio.h>
#include <math.h>
#include "core/kstring.h" 
#include "cube.h" // just for testing

#define CAMERA_SPEED 0.5f
#define CAMERA_ROTATION_SPEED 1.5f

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
            // Move camera left (strafe)
            float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
            state->camera_position.x -= CAMERA_SPEED * cosf(yaw_rad);
            // state->camera_position.z -= CAMERA_SPEED * sinf(yaw_rad);
            INFO("Camera moved left: %.2f, %.2f, %.2f", 
                 state->camera_position.x, 
                 state->camera_position.y, 
                 state->camera_position.z);
            return TRUE;
        }
        else if (key_code == 'd' || key_code == 'D')
        {
            // Move camera right (strafe)
            float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
            state->camera_position.x += CAMERA_SPEED * cosf(yaw_rad);
            // state->camera_position.z += CAMERA_SPEED * sinf(yaw_rad);
            INFO("Camera moved right: %.2f, %.2f, %.2f", 
                 state->camera_position.x, 
                 state->camera_position.y, 
                 state->camera_position.z);
            return TRUE;
        }
        else if (key_code == 'w' || key_code == 'W')
        {
            // Move camera forward
            float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
            // state->camera_position.x += CAMERA_SPEED * sinf(yaw_rad);
            state->camera_position.z -= CAMERA_SPEED * cosf(yaw_rad);
            INFO("Camera moved forward: %.2f, %.2f, %.2f", 
                 state->camera_position.x, 
                 state->camera_position.y, 
                 state->camera_position.z);
            return TRUE;
        }
        else if (key_code == 's' || key_code == 'S')
        {
            // Move camera backward
            float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
            // state->camera_position.x -= CAMERA_SPEED * sinf(yaw_rad);
            state->camera_position.z += CAMERA_SPEED * cosf(yaw_rad);
            INFO("Camera moved backward: %.2f, %.2f, %.2f", 
                 state->camera_position.x, 
                 state->camera_position.y, 
                 state->camera_position.z);
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
        if(state->mouse_pressed)
        {
            // Calculate delta movement since last position
            f32 delta_x = (f32)(x - state->last_mouse_x);
            f32 delta_y = (f32)(y - state->last_mouse_y);


            // Apply sensitivity factor to make rotation more controllable
            const f32 sensitivity = 0.5f;
            state->camera_rotation.y += delta_x * sensitivity; // Left/right movement rotates around Y axis
            state->camera_rotation.x += delta_y * sensitivity; // Up/down movement rotates around X axis
            
            // Clamp X rotation to prevent camera flipping
            if (state->camera_rotation.x > 89.0f) state->camera_rotation.x = 89.0f;
            if (state->camera_rotation.x < -89.0f) state->camera_rotation.x = -89.0f;
            
            // Normalize Y rotation
            while (state->camera_rotation.y > 360.0f) state->camera_rotation.y -= 360.0f;
            while (state->camera_rotation.y < 0.0f) state->camera_rotation.y += 360.0f;
             
        }
        state->last_mouse_x = x;
        state->last_mouse_y = y;
        break;
    }
    case EVENT_CODE_MOUSE_WHEEL:
    {
        u16 x_delta = context.data.u16[0];
        float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
        //Random hardcoded value for mouse wheel
        if (x_delta > 250) {
            state->camera_position.z += CAMERA_SPEED * cosf(yaw_rad);
        }else {
            // Move camera forward
            state->camera_position.z -= CAMERA_SPEED * cosf(yaw_rad);
        }
        break;
    }
    case EVENT_CODE_BUTTON_PRESSED:
    {
        u16 button = context.data.u16[0];
        if(button == SDL_BUTTON_LEFT)
        {
            state->mouse_pressed = TRUE;

            if (checkIfMeshSelected(state, state->last_mouse_x, state->last_mouse_y)) {

            }
        }
        else if(button == SDL_BUTTON_RIGHT) {

        }
        break;
    }
    case EVENT_CODE_BUTTON_RELEASED:
    {
        u16 button = context.data.u16[0];
        if(button == SDL_BUTTON_LEFT)
        {
            state->mouse_pressed = FALSE;
        }
        else if(button == SDL_BUTTON_RIGHT) {

        }
        break;
    }
        default: ;
    }
    return FALSE;
}


// Helper function to project 3D world position to 2D screen coordinates
void worldToScreen(vec3 worldPos, vec3 cameraPos, vec3 cameraRot, u32 screenWidth, u32 screenHeight, f32* outX, f32* outY) {
    // Calculate relative position to camera
    vec3 relative = {
        .x = worldPos.x - cameraPos.x,
        .y = worldPos.y - cameraPos.y,
        .z = worldPos.z - cameraPos.z
    };
    
    // In OpenGL, the camera looks down the negative Z axis
    // So we need to negate Z to get the correct view space
    relative.z = -relative.z;
    
    // Apply camera rotation (inverse transform)
    // Convert rotation to radians
    f32 pitchRad = -cameraRot.x * 3.14159f / 180.0f;
    f32 yawRad = -cameraRot.y * 3.14159f / 180.0f;
    
    // Rotate around Y axis (yaw)
    f32 cosY = cosf(yawRad);
    f32 sinY = sinf(yawRad);
    f32 tempX = relative.x * cosY + relative.z * sinY;
    f32 tempZ = -relative.x * sinY + relative.z * cosY;
    relative.x = tempX;
    relative.z = tempZ;
    
    // Rotate around X axis (pitch)
    f32 cosX = cosf(pitchRad);
    f32 sinX = sinf(pitchRad);
    f32 tempY = relative.y * cosX - relative.z * sinX;
    tempZ = relative.y * sinX + relative.z * cosX;
    relative.y = tempY;
    relative.z = tempZ;
    
    // Check if object is behind camera (negative Z in view space)
    if (relative.z <= 0.1f) {
        *outX = -10000; // Behind camera
        *outY = -10000;
        return;
    }
    
    // Perspective projection
    f32 fov = 45.0f * 3.14159f / 180.0f;
    f32 aspect = (f32)screenWidth / (f32)screenHeight;
    f32 tanHalfFov = tanf(fov / 2.0f);
    
    // Project to normalized device coordinates (-1 to 1)
    f32 ndcX = relative.x / (relative.z * tanHalfFov * aspect);
    f32 ndcY = relative.y / (relative.z * tanHalfFov);
    
    // Convert to screen coordinates (0 to width/height)
    *outX = (ndcX * 0.5f + 0.5f) * screenWidth;
    *outY = (0.5f - ndcY * 0.5f) * screenHeight;
}

b8 checkIfMeshSelected(game_state *state, u16 mouseX, u16 mouseY) {
    u64 mesh_count = darray_length(state->mesh_commands);
    
    // Get screen dimensions (you may need to pass these in or store them in state)
    u32 screenWidth = 1280;  // TODO: Get from window/state
    u32 screenHeight = 720;
    
    INFO("Checking %llu meshes for selection at mouse pos (%d, %d)", mesh_count, mouseX, mouseY);

    for (u64 i = 0; i < mesh_count; ++i) {
        mesh_command *cmd = &state->mesh_commands[i];
        
        // Project mesh's center to 2D screen coordinates
        f32 centerX, centerY;
        worldToScreen(cmd->position, state->camera_position, state->camera_rotation, 
                     screenWidth, screenHeight, &centerX, &centerY);
        
        // Check if center is behind camera
        if (centerX < -5000) {
            INFO("Mesh %llu is behind camera, skipping", i);
            continue;
        }
        
        // Project the corners of the mesh's bounding box to get screen-space bounds
        // For a cube centered at position with scale, corners are at ±scale from center
        vec3 corners[8];
        f32 halfSize = 1.0f; // Cube goes from -1 to +1, so half-size is 1
        
        // Generate 8 corners of the bounding box
        corners[0] = (vec3){{cmd->position.x - cmd->scale.x * halfSize, cmd->position.y - cmd->scale.y * halfSize, cmd->position.z - cmd->scale.z * halfSize}};
        corners[1] = (vec3){{cmd->position.x + cmd->scale.x * halfSize, cmd->position.y - cmd->scale.y * halfSize, cmd->position.z - cmd->scale.z * halfSize}};
        corners[2] = (vec3){{cmd->position.x - cmd->scale.x * halfSize, cmd->position.y + cmd->scale.y * halfSize, cmd->position.z - cmd->scale.z * halfSize}};
        corners[3] = (vec3){{cmd->position.x + cmd->scale.x * halfSize, cmd->position.y + cmd->scale.y * halfSize, cmd->position.z - cmd->scale.z * halfSize}};
        corners[4] = (vec3){{cmd->position.x - cmd->scale.x * halfSize, cmd->position.y - cmd->scale.y * halfSize, cmd->position.z + cmd->scale.z * halfSize}};
        corners[5] = (vec3){{cmd->position.x + cmd->scale.x * halfSize, cmd->position.y - cmd->scale.y * halfSize, cmd->position.z + cmd->scale.z * halfSize}};
        corners[6] = (vec3){{cmd->position.x - cmd->scale.x * halfSize, cmd->position.y + cmd->scale.y * halfSize, cmd->position.z + cmd->scale.z * halfSize}};
        corners[7] = (vec3){{cmd->position.x + cmd->scale.x * halfSize, cmd->position.y + cmd->scale.y * halfSize, cmd->position.z + cmd->scale.z * halfSize}};
        
        // Project all corners and find min/max screen coordinates
        f32 minScreenX = 999999.0f, maxScreenX = -999999.0f;
        f32 minScreenY = 999999.0f, maxScreenY = -999999.0f;
        
        for (int c = 0; c < 8; c++) {
            f32 sx, sy;
            worldToScreen(corners[c], state->camera_position, state->camera_rotation, 
                         screenWidth, screenHeight, &sx, &sy);
            
            // Skip corners behind camera
            if (sx < -5000) continue;
            
            if (sx < minScreenX) minScreenX = sx;
            if (sx > maxScreenX) maxScreenX = sx;
            if (sy < minScreenY) minScreenY = sy;
            if (sy > maxScreenY) maxScreenY = sy;
        }
        
        // Add padding to make selection easier (50 pixels on each side)
        f32 padding = 50.0f;
        minScreenX -= padding;
        maxScreenX += padding;
        minScreenY -= padding;
        maxScreenY += padding;
        
        // Debug output
        INFO("Mesh %llu: World pos (%.1f, %.1f, %.1f) -> Center (%.1f, %.1f)", 
             i, cmd->position.x, cmd->position.y, cmd->position.z, centerX, centerY);
        INFO("  Screen bounds (with padding): X[%.1f to %.1f], Y[%.1f to %.1f]", 
             minScreenX, maxScreenX, minScreenY, maxScreenY);
        INFO("  Mouse at: (%.1f, %.1f)", (f32)mouseX, (f32)mouseY);
        
        // Check if mouse is within the bounding box
        if (mouseX >= minScreenX && mouseX <= maxScreenX && 
            mouseY >= minScreenY && mouseY <= maxScreenY) {
            INFO(">>> MESH %llu SELECTED! (within bounding box) <<<", i);
            return TRUE;
        }
    }
    
    INFO("No mesh selected");
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

void tilt_camera(game_state *state, f32 delta_time)
{
    // Set a fixed 45-degree tilt looking downward 
    state->camera_position = (vec3){{0.0f, 5.0f, 5.0f}}; // Position camera higher and slightly back
    state->camera_rotation.x = 45.0f;                     // Look down at 45 degrees
    state->camera_rotation.y = 10.0f;                     // Slight rotation on Y for perspective
    state->camera_rotation.z = 0.0f;                      // No roll
    
    INFO("Camera position: %.2f, %.2f, %.2f", 
        state->camera_position.x, 
        state->camera_position.y, 
        state->camera_position.z);
}

void rotate_camera(game_state *state, f32 x, f32 y)
{
    // Calculate mouse movement delta from last position
    f32 delta_x = (f32)(x - state->last_mouse_x);
    f32 delta_y = (f32)(y - state->last_mouse_y);
    
    // Apply sensitivity factor to make rotation more controllable
    const f32 sensitivity = 0.2f;
    state->camera_rotation.y += delta_x * sensitivity; // Left/right movement rotates around Y axis
    state->camera_rotation.x += delta_y * sensitivity; // Up/down movement rotates around X axis
    
    // Clamp X rotation to prevent camera flipping
    if (state->camera_rotation.x > 89.0f) state->camera_rotation.x = 89.0f;
    if (state->camera_rotation.x < -89.0f) state->camera_rotation.x = -89.0f;
    
    // Normalize Y rotation
    while (state->camera_rotation.y > 360.0f) state->camera_rotation.y -= 360.0f;
    while (state->camera_rotation.y < 0.0f) state->camera_rotation.y += 360.0f;
    
    // Calculate forward direction vector for debugging
    #if 0   
    float yaw_rad = state->camera_rotation.y * 3.14159f / 180.0f;
    float pitch_rad = state->camera_rotation.x * 3.14159f / 180.0f;
    float x_dir = sinf(yaw_rad) * cosf(pitch_rad);
    float y_dir = -sinf(pitch_rad);
    float z_dir = -cosf(yaw_rad) * cosf(pitch_rad);
    #endif
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
    state->mouse_pressed = FALSE;

    // Initialize camera with logical position behind and above the cubes
    state->camera_position = (vec3){{0.0f, 3.0f, 20.0f}}; // Behind and above looking toward origin
    state->camera_rotation = (vec3){{15.0f, 0.0f, 0.0f}}; // 15-degree downward tilt
    state->last_mouse_x = 0;
    state->last_mouse_y = 0;

    // Initialize mesh and text commands arrays
    state->mesh_commands = darray_create(mesh_command);
    state->text_commands = darray_create(text_command);
    state->model_commands = darray_create(model_command);

    // Register for keyboard and mouse events
    event_register(EVENT_CODE_KEY_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_KEY_RELEASED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_MOVED, state, game_on_event);
    event_register(EVENT_CODE_MOUSE_WHEEL, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_PRESSED, state, game_on_event);
    event_register(EVENT_CODE_BUTTON_RELEASED, state, game_on_event);

    // TODO: Move this to a separate function
    //  Create mesh with 36 vertices (6 faces, 2 triangles per face, 3 vertices per triangle)
    mesh *cube_mesh = renderer_create_mesh(cube_vertices, 36);
    if (!cube_mesh)
    {
        ERROR("Failed to create cube mesh!");
        return FALSE;
    }
    mesh *cube_mesh2 = renderer_create_mesh(cube_vertices, 36);
    if (!cube_mesh2)
    {
        ERROR("Failed to create cube mesh!");
        return FALSE;
    }
    // Create and add cube mesh commands to the array
    add_mesh_to_render_packet(state, cube_mesh, 
                (vec3){{0.0f, 0.0f, 0.0f}},            // position at origin (0,0,0)
                (vec3){{0.0f, 45.0f, 0.0f}},           // Y-axis rotation for visibility
                (vec3){{1.0f, 1.0f, 1.0f}},            // uniform scale
                (vec4){{1.0f, 0.0f, 0.0f, 1.0f}});     // bright red color
                
    add_mesh_to_render_packet(state, cube_mesh2,
                (vec3){{-3.0f, 0.0f, 0.0f}},           // positioned 3 units left of origin
                (vec3){{0.0f, 0.0f, 0.0f}},            // no rotation
                (vec3){{1.0f, 2.0f, 1.0f}},            // taller cube
                (vec4){{0.0f, 1.0f, 0.0f, 1.0f}});     // bright green color

    // Create a new font
    state->font = renderer_create_font("assets/fonts/NotoMono-Regular.ttf", 18);
    if (!state->font) {
        ERROR("Failed to create font!");
        return FALSE;
    }

    // Create a new model
    // state->model_commands->model = renderer_create_model("assets/models/plane.obj");
    // if (!state->model_commands->model) {
    //     ERROR("Failed to create model!");
    //     return FALSE;
    // }
    
    // Position the plane model in a visible location
    // These will be directly used in the renderer_draw_model function
    // add_model_to_render_packet(state, state->model_commands->model,
    //             (vec3){{-5.0f, -1.0f, 0.0f}},    // Position in front of camera and slightly below
    //             (vec3){{0.0f, 0.0f, 0.0f}},      // No rotation
    //             (vec3){{2.0f, 2.0f, 2.0f}},      // Larger scale to make it more visible
    //             (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}); // White color to show texture properly
    //
    return TRUE;
}

b8 game_update(game *game_instance, f32 delta_time)
{
    game_state *state = (game_state *)game_instance->state;
    if (!state)
        return FALSE;

    state->delta_time = delta_time;
    state->fps = 1.0f / delta_time;
    char text[128]; 
    sprintf(text, "FPS: %.1f", state->fps);
    render_text(state, 0, text, (vec2){{20.0f, 50.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    sprintf(text, "Camera Position: (%.1f, %.1f, %.1f)",
            state->camera_position.x, state->camera_position.y, state->camera_position.z);
    render_text(state, 1, text, (vec2){{20.0f, 70.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    sprintf(text, "Camera Rotation: (%.1f, %.1f, %.1f)",
              state->camera_rotation.x, state->camera_rotation.y, state->camera_rotation.z);
    render_text(state, 2, text, (vec2){{20.0f, 90.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    sprintf(text, "Model Count: %llu", darray_length(state->model_commands));
    render_text(state, 3, text, (vec2){{20.0f, 110.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    sprintf(text, "Model Position: (%.1f, %.1f, %.1f)",
            state->model_commands->position.x, state->model_commands->position.y, state->model_commands->position.z);
    render_text(state, 4, text, (vec2){{20.0f, 130.0f}}, (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}, 1.0f, state->font);

    // Update all mesh positions based on their IDs
    // for (u64 i = 0; i < darray_length(state->mesh_commands); i++) {
    //     if (state->mesh_commands[i].mesh->id == 1)
    //     {
    //         // update_mesh_rotation(state, delta_time, state->mesh_commands[i].mesh->id);
    //     }
    // }

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
        
        // Debug output
        #if 0
        INFO("Rendering %llu meshes", mesh_count);
        INFO("Camera position: (%.2f, %.2f, %.2f) rotation: (%.2f, %.2f, %.2f)",
             state->camera_position.x, state->camera_position.y, state->camera_position.z,
             state->camera_rotation.x, state->camera_rotation.y, state->camera_rotation.z);
        #endif
    }
    else
    {
        current_packet.mesh_commands.count = 0;
        current_packet.mesh_commands.commands = NULL;
        // ERROR("No meshes to render!");
    }

    // Set text commands from the darray
    u64 text_count = darray_length(state->text_commands);
    if (text_count > 0)
    {
        current_packet.text_commands.commands = state->text_commands;
        current_packet.text_commands.count = (u32)text_count;
    }
    else
    {
        current_packet.text_commands.count = 0;
        current_packet.text_commands.commands = NULL;
        // ERROR("No text to render!");
    }

    u64 model_count = darray_length(state->model_commands);
    if (model_count > 0)
    {
        current_packet.model_commands.commands = state->model_commands;
        current_packet.model_commands.count = (u32)model_count;
    }
    else
    {
        current_packet.model_commands.count = 0;
        current_packet.model_commands.commands = NULL;
        // ERROR("No models to render!");
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

void add_mesh_to_render_packet(game_state *state, mesh *mesh, vec3 position, vec3 rotation, vec3 scale, vec4 color)
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

void add_model_to_render_packet(game_state *state, model *model, vec3 position, vec3 rotation, vec3 scale, vec4 color)
{
    model_command model_cmd = {
        .model = model,
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .color = color};
    darray_push(state->model_commands, model_cmd);
    INFO("Added model to render list. Position: (%.2f, %.2f, %.2f), Total count: %llu",
         position.x, position.y, position.z, darray_length(state->model_commands));
}

void render_text(game_state *state, u32 id,  const char *text, vec2 position, vec4 color, f32 scale, font *font)
{
    // Create a persistent copy of the string
    char *text_copy = string_duplicate(text);
    if (!text_copy) {
        ERROR("Failed to allocate memory for text");
        return;
    }

    u64 text_count = darray_length(state->text_commands);
    b8 found = FALSE;
    text_command text_cmd = {
        .text = text_copy,
        .id = id,
        .position = position,
        .color = color,
        .scale = scale,
        .font = font
    };

    for (u64 i = 0; i < text_count; i++)
    {
        if (state->text_commands[i].id == id)
        {
            state->text_commands[i] = text_cmd;
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        darray_push(state->text_commands, text_cmd);
    }

}