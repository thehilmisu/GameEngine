#ifndef GAME_H
#define GAME_H

#include "renderer/renderer_frontend.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_types.inl"
#include "game_types.h"

typedef struct game_state {
    f32 delta_time;
    vec4 clear_color;  // Color to clear the screen with
    mesh* triangle_mesh;
    f32 rotation;  // Add rotation state
} game_state;

b8 game_initialize(game* game_instance);
b8 game_update(game* game_instance, f32 delta_time);
b8 game_render(game* game_instance, f32 delta_time);
void game_on_resize(game* game_instance, u32 width, u32 height);
void game_shutdown(game* game_instance);

#endif
