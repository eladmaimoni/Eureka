#include "logging.hpp"


namespace eureka
{
    void InitializeDefaultLogger()
    {
        g_default_logger = spdlog::default_logger_raw();
    }
}