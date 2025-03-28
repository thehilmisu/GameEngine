#ifndef GAME_H
#define GAME_H

#include "renderer/renderer_frontend.h"
#include "core/kmemory.h"
#include "core/logger.h"
#include "core/event.h"
#include "renderer/renderer_types.inl"
#include "game_types.h"
#include <SDL2/SDL_keycode.h>  // For SDLK_* constants

typedef struct game_state {
    f32 delta_time;
    f32 fps;
    vec4 clear_color;  // Color to clear the screen with
    mesh* triangle_mesh;
    u32 triangle_mesh_id;
    f32 rotation;  // Current rotation angle
    f32 rotation_speed;  // Speed of rotation in radians per second
    
    // Camera
    vec3 camera_position;
    vec3 camera_rotation;
    u16 last_mouse_x;
    u16 last_mouse_y;

    // Text rendering
    font* default_font;
} game_state;

b8 game_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 game_initialize(game* game_instance);
b8 game_update(game* game_instance, f32 delta_time);
b8 game_render(game* game_instance, f32 delta_time);
void game_on_resize(game* game_instance, u32 width, u32 height);
void game_shutdown(game* game_instance);

#endif
