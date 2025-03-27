#pragma once

#include <core/application.h>
#include <core/event.h>
#include <core/math_types.h>

typedef struct game {
    //application configuration
    application_config app_config;

    // fp to game init
    b8 (*initialize)(struct game* game_instance);

    // fp to game update
    b8 (*update)(struct game* game_instance, f32 delta_time);

    // fp to game render
    b8 (*render)(struct game* game_instance, f32 delta_time);

    // fp to window on resize function
    void (*onresize)(struct game* game_instance, u32 width, u32 height);

    // fp to game on event
    b8 (*on_event)(struct game* game_instance, u16 code, void* sender, void* listener_inst, event_context context);

    void* state;

} game;
