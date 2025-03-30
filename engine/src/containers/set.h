#pragma once

#include "definitions.h"

/*
Memory layout
u64 capacity = number of buckets
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
u64* buckets = array of bucket indices
void* elements = array of elements
*/

enum {
    SET_CAPACITY,
    SET_LENGTH,
    SET_STRIDE,
    SET_FIELD_LENGTH
};

// Bucket states
#define SET_BUCKET_EMPTY 0xFFFFFFFF
#define SET_BUCKET_DELETED 0xFFFFFFFE

API void* _set_create(u64 capacity, u64 stride);
API void _set_destroy(void* set);

API u64 _set_field_get(void* set, u64 field);
API void _set_field_set(void* set, u64 field, u64 value);

API b8 _set_insert(void* set, const void* value_ptr);
API b8 _set_remove(void* set, const void* value_ptr);
API b8 _set_contains(void* set, const void* value_ptr);
API void* _set_find(void* set, const void* value_ptr);

#define SET_DEFAULT_CAPACITY 16
#define SET_RESIZE_FACTOR 2
#define SET_MAX_LOAD_FACTOR 0.75f

#define set_create(type) \
    _set_create(SET_DEFAULT_CAPACITY, sizeof(type))

#define set_reserve(type, capacity) \
    _set_create(capacity, sizeof(type))

#define set_destroy(set) _set_destroy(set)

#define set_insert(set, value)           \
    {                                    \
        typeof(value) temp = value;      \
        _set_insert(set, &temp);         \
    }

#define set_remove(set, value)           \
    {                                    \
        typeof(value) temp = value;      \
        _set_remove(set, &temp);         \
    }

#define set_contains(set, value)         \
    ({                                   \
        typeof(value) temp = value;      \
        _set_contains(set, &temp);       \
    })

#define set_find(set, value)             \
    ({                                   \
        typeof(value) temp = value;      \
        _set_find(set, &temp);           \
    })

#define set_clear(set) \
    _set_field_set(set, SET_LENGTH, 0)

#define set_capacity(set) \
    _set_field_get(set, SET_CAPACITY)

#define set_length(set) \
    _set_field_get(set, SET_LENGTH)

#define set_stride(set) \
    _set_field_get(set, SET_STRIDE) 