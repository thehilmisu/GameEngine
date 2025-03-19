#pragma once

#include <core/application.h>

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

    void* state;

} game;
