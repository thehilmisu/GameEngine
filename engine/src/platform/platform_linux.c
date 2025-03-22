#include "platform.h"
#include <SDL2/SDL_events.h>
#include <core/event.h>
#ifdef __linux__
#include <sys/time.h>

#include "core/logger.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <containers/darray.h>
#include <time.h>  // For platform_get_absolute_time and platform_sleep

// Internal state for SDL2 platform
typedef struct internal_state {
    SDL_Window* window;
    b8 running;  // Tracks if the application should continue running
} internal_state;

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        FATAL("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return FALSE;
    }

    // Create the internal state
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)plat_state->internal_state;
    if (!state) {
        FATAL("Failed to allocate internal state memory.");
        SDL_Quit();
        return FALSE;
    }

    // Create the window
    state->window = SDL_CreateWindow(
        application_name,           // Window title
        x == -1 ? SDL_WINDOWPOS_CENTERED : x,  // X position (-1 for centered)
        y == -1 ? SDL_WINDOWPOS_CENTERED : y,  // Y position (-1 for centered)
        width,                      // Width
        height,                     // Height
        SDL_WINDOW_SHOWN            // Flags (visible window)
    );

    if (!state->window) {
        FATAL("Window could not be created! SDL_Error: %s", SDL_GetError());
        free(state);
        SDL_Quit();
        return FALSE;
    }

    state->running = TRUE; 
    return TRUE;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    if (state) {
        if (state->window) {
            SDL_DestroyWindow(state->window);
        }
        free(state);
        plat_state->internal_state = NULL;
    }
    SDL_Quit();  
}

b8 platform_pump_messages(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:  // Window close button or Alt+F4
                state->running = FALSE;
                break;

            case SDL_KEYDOWN: {
                SDL_Scancode code = event.key.keysym.scancode;
                SDL_Keycode keycode = SDL_GetKeyFromScancode(code);
                // Fire off an event for immediate processing.
                event_context context;
                context.data.u16[0] = keycode;
                event_fire(EVENT_CODE_KEY_PRESSED, 0, context);
                break;
            }
            case SDL_KEYUP: {
                SDL_Scancode code = event.key.keysym.scancode;
                SDL_Keycode keycode = SDL_GetKeyFromScancode(code);
                // Fire off an event for immediate processing.
                event_context context;
                context.data.u16[0] = keycode;
                event_fire(EVENT_CODE_KEY_RELEASED, 0, context);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:{
                // Fire the event.
                u8 button = event.button.button;
                event_context context;
                context.data.u16[0] = button;
                event_fire(EVENT_CODE_BUTTON_PRESSED, 0, context);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                // Fire the event.
                u8 button = event.button.button;
                event_context context;
                context.data.u16[0] = button;
                event_fire(EVENT_CODE_BUTTON_RELEASED, 0, context);
                break;
            }

            case SDL_MOUSEMOTION: {
                int x = event.motion.x, y = event.motion.y;
                // Fire the event.
                event_context context;
                context.data.u16[0] = x;
                context.data.u16[1] = y;
                event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
                break;
            }

            case SDL_MOUSEWHEEL: {
                // Fire the event.
                event_context context;
                context.data.u16[0] = event.wheel.y;
                event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
                break;
            }
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // TODO: Handle resizing
                    // Example: int w = event.window ata1, h = event.window.data2;
                }
                break;

            default:
                // Ignore other events
                break;
        }
    }

    return state->running;  // Return TRUE if still running, FALSE if quitting
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);  
}

void platform_free(void* block, b8 aligned) {
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platform_console_write(const char* message, log_level level) {
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[level], message);
}

void platform_console_write_error(const char* message, log_level level) {
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[level], message);
}

f64 platform_get_absolute_time() {
    Uint64 ticks = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    return (f64)ticks / (f64)freq;  // Time in seconds
}
void platform_sleep(u64 ms) {
    SDL_Delay(ms);  // SDL2’s built-in delay function
}

void platform_get_required_extension_names(const char ***names_darray) {
     darray_push(*names_darray, &"VK_KHR_xcb_surface");  // VK_KHR_xlib_surface?
}
#endif
