#pragma once

#include "core/logger.h"
#include "definitions.h"
#include <SDL2/SDL.h>

typedef struct platform_state{
    void* internal_state;
    SDL_Window* window_handle;  // SDL window handle
} platform_state;

b8 platform_startup(
    platform_state* plat_state,
    const char* appliation_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height);

void platform_shutdown(platform_state* plat_state);

b8 platform_pump_messages(platform_state* plat_state);

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

void platform_console_write(const char* message, log_level level);
void platform_console_write_error(const char* message, log_level level);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);

b8 platform_file_exists(const char* path);
b8 platform_create_directory(const char* path);
b8 platform_delete_file(const char* path);
u64 platform_get_file_size(const char* path);
b8 platform_read_file_to_string(const char* path, char** buffer, u64* size);
b8 platform_write_string_to_file(const char* path, const char* string);
b8 platform_read_file_to_buffer(const char* path, char** buffer, u64* size);
b8 platform_write_buffer_to_file(const char* path, const char* buffer, u64 size);