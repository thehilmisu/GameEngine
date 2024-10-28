#ifndef LOG_H
#define LOG_H

#include "spdlog/spdlog.h"

class Log {
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {return s_ClientLogger;}

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;

};

#define     CORE_ERROR(...)     Log::GetCoreLogger()->error(__VA_ARGS__)
#define     CORE_WARN(...)      Log::GetCoreLogger()->warn(__VA_ARGS__)
#define     CORE_INFO(...)      Log::GetCoreLogger()->info(__VA_ARGS__)
#define     CORE_TRACE(...)     Log::GetCoreLogger()->trace(__VA_ARGS__)
#define     CORE_FATAL(...)     Log::GetCoreLogger()->fatal(__VA_ARGS__)


#define     CLIENT_ERROR(...)     Log::GetClientLogger()->error(__VA_ARGS__)
#define     CLIENT_WARN(...)      Log::GetClientLogger()->warn(__VA_ARGS__)
#define     CLIENT_INFO(...)      Log::GetClientLogger()->info(__VA_ARGS__)
#define     CLIENT_TRACE(...)     Log::GetClientLogger()->trace(__VA_ARGS__)
#define     CLIENT_FATAL(...)     Log::GetClientLogger()->fatal(__VA_ARGS__)


#endif // LOG_H
