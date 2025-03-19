#include "application.h"
#include "definitions.h"
#include "game_types.h"
#include <core/logger.h>
#include <platform/platform.h>

typedef struct application_state {
    game* game_instance;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_create(game* game_instance){

    if(initialized){
        ERROR("application_create called more than once");
        return FALSE;
    }

    app_state.game_instance = game_instance;
    // called but for now useless
    initialize_logging();

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;
    
    if(!platform_startup(&app_state.platform, game_instance->app_config.name, 
                        game_instance->app_config.start_pos_x, game_instance->app_config.start_pos_y, 
                        game_instance->app_config.start_width, game_instance->app_config.start_height)){
        return FALSE;
    }

    if(!app_state.game_instance->initialize(app_state.game_instance)){
        FATAL("Game failed to initialize!");
        return FALSE;
    }

    app_state.game_instance->onresize(app_state.game_instance, app_state.width, app_state.height);

    initialized = TRUE;
    return TRUE;
}

b8 application_run(){
    while(app_state.is_running){
        if(!platform_pump_messages(&app_state.platform)){ app_state.is_running = FALSE;}
        
        if(!app_state.is_suspended){
            if(!app_state.game_instance->update(app_state.game_instance, (f32)0)){
                FATAL("Game update failed, shutting down");
                app_state.is_running = FALSE;
                break;
            }

            if(!app_state.game_instance->render(app_state.game_instance, (f32)0)){
                FATAL("Game render failed, shutting down");
                app_state.is_running = FALSE;
                break;
            }
        }
    }
    app_state.is_running = FALSE;
    platform_shutdown(&app_state.platform);
    return TRUE;
}
