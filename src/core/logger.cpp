#include "logger.h"

#include <iostream>

namespace shooter
{

Logger::Logger()
{
    try
    {
        m_logger = spdlog::stdout_color_mt("shooter");
        m_logger->set_pattern("%^[%H:%M:%S][%n(%l)]:%v%$");
        m_logger->set_level(spdlog::level::trace);
    }
    catch(const spdlog::spdlog_ex& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

}    
