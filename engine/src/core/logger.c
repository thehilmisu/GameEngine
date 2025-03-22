#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: you should make that platform specific
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>



void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
    log_output(
        LOG_LEVEL_FATAL, 
        "Assertion failure: %s, message: %s, in file : %s, line: %d", 
        expression, 
        message,
        file,
        line);
}

b8 initialize_logging(){
    // TODO: create log file
    return TRUE;
}
void shutdown_logging(){
    // TODO: cleanup logging/write queued entries
}

void log_output(log_level level, const char* message, ...){
    const char* level_strings[6] = 
        {
        "[FATAL]   : ", 
        "[ERROR]   : ", 
        "[WARNING] : ", 
        "[INFO]    : ", 
        "[DEBUG]   : ", 
        "[TRACE]   : "
        };
    b8 is_error = level < 2;

    i32 size = 32000;
    char out_message[size];
    memset(out_message, 0, sizeof(out_message));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, size, message, arg_ptr);
    va_end(arg_ptr);

    char m[size];
    sprintf(m, "%s%s\n", level_strings[level], out_message);

    if(is_error){
        platform_console_write_error(m, level);
    }else{
        platform_console_write(m,level);
    }
}


