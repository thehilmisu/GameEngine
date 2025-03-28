#include "application.h"
#include "definitions.h"
#include "game_types.h"
#include <core/logger.h>
#include <platform/platform.h>
#include <core/kmemory.h>
#include <core/event.h>
#include <core/clock.h>
#include <SDL2/SDL_keycode.h>
#include "renderer/renderer_frontend.h"

typedef struct application_state {
    game* game_instance;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

// Event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_create(game* game_instance){
   
    // called but for now useless
    initialize_logging();

    if(initialized){
        ERROR("application_create called more than once");
        return FALSE;
    }

    app_state.game_instance = game_instance;
    

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if(!event_initialize()){
        ERROR("Event system failed to initialize!");
        return FALSE;
    }
    
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    
    // Set initial window size
    app_state.width = game_instance->app_config.start_width;
    app_state.height = game_instance->app_config.start_height;

    if(!platform_startup(&app_state.platform, game_instance->app_config.name, 
                        game_instance->app_config.start_pos_x, game_instance->app_config.start_pos_y, 
                        game_instance->app_config.start_width, game_instance->app_config.start_height)){
        return FALSE;
    }
    
    // Renderer startup
    if (!renderer_initialize(game_instance->app_config.name, &app_state.platform)) {
       FATAL("Failed to initialize renderer. Aborting application.");
       return FALSE;
    }

    if(!app_state.game_instance->initialize(app_state.game_instance)){
        FATAL("Game failed to initialize!");
        return FALSE;
    }

    // Send initial resize event
    event_context context;
    context.data.u16[0] = app_state.width;
    context.data.u16[1] = app_state.height;
    event_fire(EVENT_CODE_RESIZED, 0, context);

    initialized = TRUE;
    return TRUE;
}

b8 application_run(){
    
    INFO(get_memory_usage_str());

    clock_start(&app_state.clock);
    clock_update(&app_state.clock);

    app_state.last_time = app_state.clock.elapsed;
    f64 runing_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f/60;

    while(app_state.is_running){
        if(!platform_pump_messages(&app_state.platform)){ app_state.is_running = FALSE;}
        
        if(!app_state.is_suspended){
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta = (current_time - app_state.last_time);
            f64 frame_start_time = platform_get_absolute_time();

            if(!app_state.game_instance->update(app_state.game_instance, (f32)delta)){
                FATAL("Game update failed, shutting down");
                app_state.is_running = FALSE;
                break;
            }

            if(!app_state.game_instance->render(app_state.game_instance, (f32)delta)){
                FATAL("Game render failed, shutting down");
                app_state.is_running = FALSE;
                break;
            }
            
            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            runing_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;
            
            if(remaining_seconds > 0)
            {
                u64 remaining_ms = (remaining_seconds * 1000);

                // Limit frame rate to avoid over cpu usage
                b8 limit_frames = TRUE;
                if(remaining_ms > 0 && limit_frames){
                    platform_sleep(remaining_ms - 1);
                }

                frame_count++;
            }

            app_state.last_time = current_time;
        }
    }
    app_state.is_running = FALSE;
    
    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
    event_shutdown();
    
    renderer_shutdown(); 
    
    platform_shutdown(&app_state.platform);
    
    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
     switch (code) {
         case EVENT_CODE_APPLICATION_QUIT: {
             INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
             app_state.is_running = FALSE;
             return TRUE;
         }
     }
 
     return FALSE;
 }
 
 b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
     if (code == EVENT_CODE_KEY_PRESSED) {
         u16 key_code = context.data.u16[0];
         if (key_code == SDLK_ESCAPE) {
             // NOTE: Technically firing an event to itself, but there may be other listeners.
             event_context data = {};
             event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
             // Block anything else from processing this.
             return TRUE;
         } else if (key_code == SDLK_a) {
             // Example on checking for a key
             DEBUG("Explicit - A key pressed!");
         } else {
             DEBUG("'%c' key pressed in window.", key_code);
         }
     } else if (code == EVENT_CODE_KEY_RELEASED) {
         u16 key_code = context.data.u16[0];
         if (key_code == SDLK_b) {
             // Example on checking for a key
             DEBUG("Explicit - B key released!");
         } else {
             DEBUG("'%c' key released in window.", key_code);
         }
     }
     return FALSE;
 }
