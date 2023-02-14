#include "FlutterUtils.hpp"
#include <stdexcept>

namespace eureka::flutter
{
    uint64_t CurrentTimeMicroseconds()
    {
        return FlutterEngineGetCurrentTime() / 1000;
    }
    void CheckFlutterResult(FlutterEngineResult result)
    {
        if (result != FlutterEngineResult::kSuccess)
        {
            throw std::runtime_error("flutter error");
        }
    }
}