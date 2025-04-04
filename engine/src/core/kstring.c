#include "kstring.h"
#include <core/kmemory.h>

#include <string.h>

u64 string_length(const char* str){
    return strlen(str);
}

char* string_duplicate(const char* str){
    u64 length = string_length(str);
    char* copy = kallocate(length + 1, MEMORY_TAG_STRING);
    kcopy_memory(copy, str, length + 1);
    return copy;
}

b8 strings_equal(const char* str1, const char* str2){
    return strcmp(str1, str2) == 0;
}
