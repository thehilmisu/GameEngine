#pragma once

// unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// floating point types
typedef float f32;
typedef double f64;

//boolean types
typedef int b32;
typedef int b8;

// Compile-time size checks
enum {
    __check_u8  = sizeof(u8)  == 1 ? 1 : -1,
    __check_u16 = sizeof(u16) == 2 ? 1 : -1,
    __check_u32 = sizeof(u32) == 4 ? 1 : -1,
    __check_u64 = sizeof(u64) == 8 ? 1 : -1,
    
    __check_i8  = sizeof(i8)  == 1 ? 1 : -1,
    __check_i16 = sizeof(i16) == 2 ? 1 : -1,
    __check_i32 = sizeof(i32) == 4 ? 1 : -1,
    __check_i64 = sizeof(i64) == 8 ? 1 : -1,
    
    __check_f32 = sizeof(f32) == 4 ? 1 : -1,
    __check_f64 = sizeof(f64) == 8 ? 1 : -1
};

#define TRUE  1
#define FALSE 0 

#define CLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max : value

#define API __attribute__((visibility("default")))

