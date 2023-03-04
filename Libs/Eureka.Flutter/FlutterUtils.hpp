#pragma once

#include <flutter/flutter_embedder.h>
#include <chrono>

namespace eureka::flutter
{
    using namespace std::chrono_literals;

    inline std::chrono::nanoseconds FRAME_INTERVAL_60_FPS_NS = std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / 60;
  
    std::chrono::nanoseconds CurrentTimeNanoseconds();
    uint64_t CurrentTimeMicroseconds();
    void CheckFlutterResult(FlutterEngineResult result);


} // namespace eureka::flutter

#define FLUTTER_CHECK(stmt) eureka::flutter::CheckFlutterResult(stmt);