#include "platform.h"
#include <SDL2/SDL_events.h>
#include <core/event.h>
#include <core/kmemory.h>
#include <core/kstring.h>
#ifdef __linux__
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <containers/darray.h>
#include <time.h>  // For platform_get_absolute_time and platform_sleep

// Internal state for SDL2 platform
typedef struct internal_state {
    SDL_Window* window;
    SDL_GLContext gl_context;
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

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create the window
    state->window = SDL_CreateWindow(
        application_name,           // Window title
        x == -1 ? SDL_WINDOWPOS_CENTERED : x,  // X position (-1 for centered)
        y == -1 ? SDL_WINDOWPOS_CENTERED : y,  // Y position (-1 for centered)
        width,                      // Width
        height,                     // Height
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE   // Flags (visible window with OpenGL support)
    );

    if (!state->window) {
        FATAL("Window could not be created! SDL_Error: %s", SDL_GetError());
        free(state);
        SDL_Quit();
        return FALSE;
    }

    // Store window handle in platform state
    plat_state->window_handle = state->window;

    // Create OpenGL context
    state->gl_context = SDL_GL_CreateContext(state->window);
    if (!state->gl_context) {
        FATAL("OpenGL context could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(state->window);
        free(state);
        SDL_Quit();
        return FALSE;
    }

    // Enable VSync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        WARN("Unable to set VSync! SDL Error: %s", SDL_GetError());
    }

    state->running = TRUE;

    // Fire initial resize event
    event_context context;
    context.data.u16[0] = width;
    context.data.u16[1] = height;
    event_fire(EVENT_CODE_RESIZED, 0, context);

    return TRUE;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    if (state) {
        if (state->gl_context) {
            SDL_GL_DeleteContext(state->gl_context);
        }
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
                    // Fire a resize event
                    event_context context;
                    context.data.u16[0] = event.window.data1;  // width
                    context.data.u16[1] = event.window.data2;  // height
                    event_fire(EVENT_CODE_RESIZED, 0, context);
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

b8 platform_file_exists(const char* path) {
    return access(path, F_OK) != -1;
}

b8 platform_create_directory(const char* path) {
    return mkdir(path, 0777) != -1;
}

b8 platform_delete_file(const char* path) {
    return unlink(path) != -1;
}

u64 platform_get_file_size(const char* path) {
    struct stat stat_buffer;
    stat(path, &stat_buffer);
    return stat_buffer.st_size;
}

b8 platform_read_file_to_string(const char* path, char** buffer, u64* size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return FALSE;
    }   

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = (char*)malloc(*size);
    fread(*buffer, 1, *size, file);
    fclose(file);

    return TRUE;
}

b8 platform_write_string_to_file(const char* path, const char* string) {
    FILE* file = fopen(path, "w");
    if (!file) {    
        return FALSE;
    }

    fwrite(string, 1, strlen(string), file);
    fclose(file);
    return TRUE;
}

b8 platform_read_file_to_buffer(const char* path, char** buffer, u64* size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return FALSE;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = (char*)malloc(*size);
    fread(*buffer, 1, *size, file);
    fclose(file);
    return TRUE;
}

b8 platform_write_buffer_to_file(const char* path, const char* buffer, u64 size) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        return FALSE;
    }

    fwrite(buffer, 1, size, file);
    fclose(file);
    return TRUE;
}
#endif
