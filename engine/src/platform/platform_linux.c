#include "platform.h"
#include <SDL2/SDL_events.h>
#include <core/event.h>
#include <core/kmemory.h>
#include <core/kstring.h>
#ifdef __linux__
#include <sys/time.h>

#include "core/logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <containers/darray.h>
#include <time.h>  // For platform_get_absolute_time and platform_sleep

#include <vulkan/vulkan.h>
#include "renderer/vulkan/vulkan_types.inl"

// Internal state for SDL2 platform
typedef struct internal_state {
    SDL_Window* window;
    b8 running;  // Tracks if the application should continue running
    VkSurfaceKHR surface;
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
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN   // Flags (visible window with Vulkan support)
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
    SDL_Delay(ms);  // SDL2's built-in delay function
}

void platform_get_required_extension_names(const char ***names_darray) {
    // Get SDL's required Vulkan extensions
    unsigned int extension_count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(NULL, &extension_count, NULL)) {
        FATAL("Failed to get SDL Vulkan extension count: %s", SDL_GetError());
        return;
    }

    DEBUG("SDL reported %d required extensions", extension_count);

    // Create the darray with the correct capacity
    *names_darray = darray_create(const char*);
    if (!*names_darray) {
        FATAL("Failed to create darray for extension names.");
        return;
    }

    // Get the extensions
    const char** extensions = kallocate(sizeof(const char*) * extension_count, MEMORY_TAG_RENDERER);
    if (!extensions) {
        FATAL("Failed to allocate memory for extension names.");
        darray_destroy(*names_darray);
        return;
    }

    if (!SDL_Vulkan_GetInstanceExtensions(NULL, &extension_count, extensions)) {
        FATAL("Failed to get SDL Vulkan extensions: %s", SDL_GetError());
        kfree(extensions, sizeof(const char*) * extension_count, MEMORY_TAG_RENDERER);
        darray_destroy(*names_darray);
        return;
    }

    // Add each extension to the darray
    for (unsigned int i = 0; i < extension_count; ++i) {
        DEBUG("Adding extension: %s", extensions[i]);
        // Store the extension name directly without taking its address
        darray_push(*names_darray, extensions[i]);
    }

    // Free the temporary array
    kfree(extensions, sizeof(const char*) * extension_count, MEMORY_TAG_RENDERER);
}

b8 platform_create_vulkan_surface(platform_state *plat_state, vulkan_context *context) {
    // Simply cold-cast to the known type.
    internal_state *state = (internal_state *)plat_state->internal_state;

    DEBUG("Creating Vulkan surface for window: %p", state->window);
    DEBUG("Vulkan instance: %p", context->instance);

    // Create the Vulkan surface using SDL
    if (!SDL_Vulkan_CreateSurface(state->window, context->instance, &state->surface)) {
        FATAL("Failed to create Vulkan surface: %s", SDL_GetError());
        return FALSE;
    }

    DEBUG("SDL Vulkan surface created successfully: %p", state->surface);
    context->surface = state->surface;
    return TRUE;
}
#endif
