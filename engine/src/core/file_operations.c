#include "file_operations.h"

#include "core/logger.h"
#include "platform/platform.h"


API b8 file_exists(const char* path) {
    return platform_file_exists(path);
}

API b8 create_directory(const char* path) {
    return platform_create_directory(path);
}   

API b8 delete_file(const char* path) {
    return platform_delete_file(path);
}

API u64 get_file_size(const char* path) {
    return platform_get_file_size(path);
}

API b8 read_file_to_string(const char* path, char** buffer, u64* size) {
    return platform_read_file_to_string(path, buffer, size);
}   

API b8 write_string_to_file(const char* path, const char* string) {
    return platform_write_string_to_file(path, string);
}

API b8 read_file_to_buffer(const char* path, char** buffer, u64* size) {
    return platform_read_file_to_buffer(path, buffer, size);
}

API b8 write_buffer_to_file(const char* path, const char* buffer, u64 size) {
    return platform_write_buffer_to_file(path, buffer, size);
}
