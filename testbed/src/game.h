#ifndef GAME_H
#define GAME_H

#include "game_types.h"
#include "renderer/renderer_types.inl"

typedef struct game_state {
    f32 delta_time;
    vec3 camera_position;
    vec3 camera_rotation;
    mesh* cube_mesh;
    mesh_command cube_cmd;
    text_command fps_cmd;
} game_state;

b8 game_initialize(game* game_instance);
b8 game_update(game* game_instance, f32 delta_time);
b8 game_render(game* game_instance, f32 delta_time);
void game_on_resize(game* game_instance, u32 width, u32 height);

#endif
