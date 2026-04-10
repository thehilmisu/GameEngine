#include "game.h"
#include "cube.h"
#include "input.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/clock.h"
#include "renderer/renderer_frontend.h"
#include "resources/texture.h"

b8 game_initialize(game* game_instance) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = 0.0f;

    // Place camera back along Z so the cube at origin is visible
    state->camera_position = (vec3){{0.0f, 0.0f, 5.0f}};
    state->camera_rotation = (vec3){{0.0f, 0.0f, 0.0f}};

    // Create a single cube mesh (36 vertices, 6 faces x 2 triangles x 3 verts)
    state->cube_mesh = renderer_create_mesh(cube_vertices, 36);
    if (!state->cube_mesh) {
        ERROR("Failed to create cube mesh!");
        return FALSE;
    }

    state->cube_cmd = (mesh_command){
        .mesh     = state->cube_mesh,
        .position = (vec3){{0.0f, 0.0f, 0.0f}},
        .rotation = (vec3){{0.0f, 0.0f, 0.0f}},
        .scale    = (vec3){{1.0f, 1.0f, 1.0f}},
        .color    = (vec4){{1.0f, 0.5f, 0.0f, 1.0f}}  // orange
    };


    state->fps_cmd = (text_command){
        .text = "",
        .position = (vec2){{10.0f, 20.0f}},
        .color = (vec4){{1.0f, 1.0f, 1.0f, 1.0f}},
        .scale = 0.5f,
        .font = renderer_get_default_font()
    };

    // model* m = kallocate(sizeof(model), MEMORY_TAG_MODEL);
    // m->texture = texture_create_default_checkerboard();
    //
    // state->model_cmd = (model_command){
    //     .model = m,
    //     .position = (vec3){{0.0f, 0.0f, 0.0f}},
    //     .rotation = (vec3){{0.0f, 0.0f, 0.0f}},
    //     .scale    = (vec3){{1.0f, 1.0f, 1.0f}},
    //     .color    = (vec4){{1.0f, 1.0f, 1.0f, 1.0f}}
    // };

    INFO("Game initialized.");

    if (!input_initialize(game_instance)) INFO("Input system could not initialize!");

    return TRUE;
}

b8 game_update(game* game_instance, f32 delta_time) {
    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    state->delta_time = delta_time;

    // Slowly spin the cube on Y axis (45 deg/sec)
    state->cube_cmd.rotation.y += 45.0f * delta_time;
    if (state->cube_cmd.rotation.y >= 360.0f)
        state->cube_cmd.rotation.y -= 360.0f;

    static char fps_text[64];
    sprintf(fps_text, "FPS: %f", state->delta_time);
    state->fps_cmd.text = fps_text;

    return TRUE;
}

b8 game_render(game* game_instance, f32 delta_time) {
    (void)delta_time; // Unused for now

    game_state* state = (game_state*)game_instance->state;
    if (!state) return FALSE;

    render_packet packet = {0};
    packet.camera_position       = state->camera_position;
    packet.camera_rotation       = state->camera_rotation;
    packet.mesh_commands.commands = &state->cube_cmd;
    packet.mesh_commands.count    = 1;
    packet.text_commands.commands = &state->fps_cmd;
    packet.text_commands.count = 1;
    // packet.model_commands.commands = &state->model_cmd;
    // packet.model_commands.count = 1;

    renderer_draw_frame(&packet);
    return TRUE;
}

void game_on_resize(game* game_instance, u32 width, u32 height) {
    (void)game_instance;
    (void)width;
    (void)height;
    // Nothing to do for now
}
