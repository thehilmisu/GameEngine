#include "set.h"
#include "core/kmemory.h"
#include "core/kstring.h"
#include <string.h>

static u64 _set_hash(const void* value_ptr, u64 stride) {
    u64 hash = 0;
    const u8* bytes = (const u8*)value_ptr;
    for (u64 i = 0; i < stride; i++) {
        hash = (hash * 31) + bytes[i];
    }
    return hash;
}

static u64 _set_get_bucket_index(void* set, const void* value_ptr) {
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    u64 hash = _set_hash(value_ptr, _set_field_get(set, SET_STRIDE));
    return hash % capacity;
}

static void* _set_get_buckets(void* set) {
    return (void*)((u8*)set + sizeof(u64) * SET_FIELD_LENGTH);
}

static void* _set_get_elements(void* set) {
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    return (void*)((u8*)_set_get_buckets(set) + sizeof(u64) * capacity);
}

static void* _set_resize(void* set) {
    u64 old_capacity = _set_field_get(set, SET_CAPACITY);
    u64 new_capacity = old_capacity * SET_RESIZE_FACTOR;
    u64 stride = _set_field_get(set, SET_STRIDE);
    
    // Create new set with increased capacity
    void* new_set = _set_create(new_capacity, stride);
    if (!new_set) return 0;
    
    // Copy elements to new set
    void* old_elements = _set_get_elements(set);
    u64 length = _set_field_get(set, SET_LENGTH);
    for (u64 i = 0; i < length; i++) {
        void* element = (void*)((u8*)old_elements + i * stride);
        _set_insert(new_set, element);
    }
    
    // Destroy old set
    _set_destroy(set);
    
    return new_set;
}

void* _set_create(u64 capacity, u64 stride) {
    u64 header_size = sizeof(u64) * SET_FIELD_LENGTH;
    u64 buckets_size = sizeof(u64) * capacity;
    u64 elements_size = stride * capacity;
    u64 total_size = header_size + buckets_size + elements_size;
    
    void* set = kallocate(total_size, MEMORY_TAG_STRING);
    if (!set) return 0;
    
    // Initialize header
    _set_field_set(set, SET_CAPACITY, capacity);
    _set_field_set(set, SET_LENGTH, 0);
    _set_field_set(set, SET_STRIDE, stride);
    
    // Initialize buckets to empty
    u64* buckets = (u64*)_set_get_buckets(set);
    for (u64 i = 0; i < capacity; i++) {
        buckets[i] = SET_BUCKET_EMPTY;
    }
    
    return set;
}

void _set_destroy(void* set) {
    if (!set) return;
    kfree(set, _set_field_get(set, SET_CAPACITY) * 
        (_set_field_get(set, SET_STRIDE) + sizeof(u64)) + 
        sizeof(u64) * SET_FIELD_LENGTH, 
        MEMORY_TAG_STRING);
}

u64 _set_field_get(void* set, u64 field) {
    return *((u64*)set + field);
}

void _set_field_set(void* set, u64 field, u64 value) {
    *((u64*)set + field) = value;
}

b8 _set_insert(void* set, const void* value_ptr) {
    if (!set || !value_ptr) return FALSE;
    
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    u64 length = _set_field_get(set, SET_LENGTH);
    u64 stride = _set_field_get(set, SET_STRIDE);
    
    // Check load factor and resize if necessary
    if ((f32)length / capacity >= SET_MAX_LOAD_FACTOR) {
        set = _set_resize(set);
        if (!set) return FALSE;
        capacity = _set_field_get(set, SET_CAPACITY);
    }
    
    u64 bucket_index = _set_get_bucket_index(set, value_ptr);
    u64* buckets = (u64*)_set_get_buckets(set);
    void* elements = _set_get_elements(set);
    
    // Linear probing
    while (buckets[bucket_index] != SET_BUCKET_EMPTY) {
        void* existing = (void*)((u8*)elements + buckets[bucket_index] * stride);
        if (memcmp(existing, value_ptr, stride) == 0) {
            return FALSE; // Element already exists
        }
        bucket_index = (bucket_index + 1) % capacity;
    }
    
    // Insert new element
    buckets[bucket_index] = length;
    void* dest = (void*)((u8*)elements + length * stride);
    memcpy(dest, value_ptr, stride);
    _set_field_set(set, SET_LENGTH, length + 1);
    
    return TRUE;
}

b8 _set_remove(void* set, const void* value_ptr) {
    if (!set || !value_ptr) return FALSE;
    
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    u64 stride = _set_field_get(set, SET_STRIDE);
    u64* buckets = (u64*)_set_get_buckets(set);
    void* elements = _set_get_elements(set);
    
    u64 bucket_index = _set_get_bucket_index(set, value_ptr);
    
    // Linear probing
    while (buckets[bucket_index] != SET_BUCKET_EMPTY) {
        void* existing = (void*)((u8*)elements + buckets[bucket_index] * stride);
        if (memcmp(existing, value_ptr, stride) == 0) {
            buckets[bucket_index] = SET_BUCKET_DELETED;
            return TRUE;
        }
        bucket_index = (bucket_index + 1) % capacity;
    }
    
    return FALSE;
}

b8 _set_contains(void* set, const void* value_ptr) {
    if (!set || !value_ptr) return FALSE;
    
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    u64 stride = _set_field_get(set, SET_STRIDE);
    u64* buckets = (u64*)_set_get_buckets(set);
    void* elements = _set_get_elements(set);
    
    u64 bucket_index = _set_get_bucket_index(set, value_ptr);
    
    // Linear probing
    while (buckets[bucket_index] != SET_BUCKET_EMPTY) {
        void* existing = (void*)((u8*)elements + buckets[bucket_index] * stride);
        if (memcmp(existing, value_ptr, stride) == 0) {
            return TRUE;
        }
        bucket_index = (bucket_index + 1) % capacity;
    }
    
    return FALSE;
}

void* _set_find(void* set, const void* value_ptr) {
    if (!set || !value_ptr) return 0;
    
    u64 capacity = _set_field_get(set, SET_CAPACITY);
    u64 stride = _set_field_get(set, SET_STRIDE);
    u64* buckets = (u64*)_set_get_buckets(set);
    void* elements = _set_get_elements(set);
    
    u64 bucket_index = _set_get_bucket_index(set, value_ptr);
    
    // Linear probing
    while (buckets[bucket_index] != SET_BUCKET_EMPTY) {
        void* existing = (void*)((u8*)elements + buckets[bucket_index] * stride);
        if (memcmp(existing, value_ptr, stride) == 0) {
            return existing;
        }
        bucket_index = (bucket_index + 1) % capacity;
    }
    
    return 0;
} 