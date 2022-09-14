#pragma once

#ifdef PERFETTO_TRACING
#include "perfetto/perfetto.h"
#endif
#include <string_view>

namespace eureka::profiling
{
#ifdef EUREKA_HAS_NVTOOLSEXT
    inline constexpr uint32_t PROFILING_CATEGORY_LOAD = 993;
    inline constexpr uint32_t PROFILING_CATEGORY_INIT = 992;
    inline constexpr uint32_t PROFILING_CATEGORY_RENDERING = 991;
    inline constexpr uint32_t PROFILING_CATEGORY_DEFAULT = 990;
#else
    inline constexpr char PROFILING_CATEGORY_LOAD[] = "load";
    inline constexpr char PROFILING_CATEGORY_INIT[] = "init";
    inline constexpr char PROFILING_CATEGORY_RENDERING[] = "rendering";
    inline constexpr char PROFILING_CATEGORY_DEFAULT[] = "default";
#endif
    void SetPerfettoThreadName(std::string_view thread_name);
}

#ifdef PERFETTO_TRACING
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category(eureka::profiling::PROFILING_CATEGORY_DEFAULT).SetDescription("default"),
    perfetto::Category(eureka::profiling::PROFILING_CATEGORY_RENDERING).SetDescription("rendering"),
    perfetto::Category(eureka::profiling::PROFILING_CATEGORY_INIT).SetDescription("system initialization"),
    perfetto::Category(eureka::profiling::PROFILING_CATEGORY_LOAD).SetDescription("asset loading"),
);
#endif