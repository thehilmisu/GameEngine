#pragma once

#include "definitions.h"
     
API b8 file_exists(const char* path);
API b8 create_directory(const char* path);
API b8 delete_file(const char* path);
API u64 get_file_size(const char* path);
API b8 read_file_to_string(const char* path, char** buffer, u64* size);
API b8 write_string_to_file(const char* path, const char* string);
API b8 read_file_to_buffer(const char* path, char** buffer, u64* size);
API b8 write_buffer_to_file(const char* path, const char* buffer, u64 size);

