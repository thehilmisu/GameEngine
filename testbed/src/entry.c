#include "game.h"
#include "entry.h"

#include <core/kmemory.h>

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
    out_game->on_key_event = game_on_event;
    out_game->on_mouse_event = game_on_event;

    // Allocate state
    out_game->state = kallocate(sizeof(game_state), MEMORY_TAG_GAME);

    return TRUE;
}
