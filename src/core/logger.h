#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// simple logger

namespace shooter
{

class Logger
{
public:
    static Logger &instance()
    {
        static Logger l;
        return l;
    }

    static std::shared_ptr<spdlog::logger> get_logger()
    {
        return instance().m_logger;
    }

private:
    Logger();
    ~Logger() = default;
private:
    std::shared_ptr<spdlog::logger> m_logger;
};

#define LOG_TRACE(...)    shooter::Logger::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     shooter::Logger::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)     shooter::Logger::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    shooter::Logger::get_logger()->error(__VA_ARGS__)
#define LOG_FATAL(...)    shooter::Logger::get_logger()->critical(__VA_ARGS__)

}

#endif // !LOGGER_H
