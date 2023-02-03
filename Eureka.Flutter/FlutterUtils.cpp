#include "FlutterUtils.hpp"
#include <stdexcept>

namespace eureka::flutter
{
    void CheckFlutterResult(FlutterEngineResult result)
    {
        if (result != FlutterEngineResult::kSuccess)
        {
            throw std::runtime_error("flutter error");
        }
    }
}