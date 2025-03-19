#pragma once

#include "definitions.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
}log_level;


b8 initialize_logging();
void shutdown_logging();

API void log_output(log_level level, const char* message, ...);

#define FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef ERROR
#define ERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define WARN(message, ...) log_output(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#else
#define WARN(message, ...)
#endif    

#if LOG_INFO_ENABLED == 1
#define INFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define INFO(message, ...)
#endif    


#if LOG_DEBUG_ENABLED == 1
#define DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define DEBUG(message, ...)
#endif    


#if LOG_TRACE_ENABLED == 1
#define TRACE(message, ...) log_output(LOG_TRACE_DEBUG, message, ##__VA_ARGS__);
#else
#define TRACE(message, ...)
#endif    



