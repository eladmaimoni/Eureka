#pragma once

#include <flutter/flutter_embedder.h>

namespace eureka::flutter
{

    void CheckFlutterResult(FlutterEngineResult result);

} // namespace eureka::flutter

#define FLUTTER_CHECK(stmt) eureka::flutter::CheckFlutterResult(stmt);