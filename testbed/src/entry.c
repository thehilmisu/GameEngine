#include "game.h"
#include "entry.h"

#include <core/kmemory.h>

// Wrapper function to match the game's event handler signature
b8 game_event_wrapper(game* game_instance, u16 code, void* sender, void* listener_inst, event_context context) {
    return game_on_event(code, sender, listener_inst, context);
}

b8 create_game(game* out_game) {
    // Application configuration
    out_game->app_config.start_pos_x = 100;
    out_game->app_config.start_pos_y = 100;
    out_game->app_config.start_width = 1280;
    out_game->app_config.start_height = 720;
    out_game->app_config.name = "Game Engine";

    // Function pointers
    out_game->initialize = game_initialize;
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->onresize = game_on_resize;
    out_game->on_event = game_event_wrapper;
    // Allocate state
    out_game->state = kallocate(sizeof(game_state), MEMORY_TAG_GAME);

    return TRUE;
}
