#include "App.hpp"
#include <debugger_trace.hpp>
#ifdef PERFETTO_TRACING
#include "perfetto_tracing_session.hpp"
#endif
#include <profiling.hpp>

using namespace eureka;


int main(int argc, char* argv[])
{
#ifdef PERFETTO_TRACING
    std::unique_ptr<eureka::PerfettoTracing> perfettoTracing;
    if (argc >= 2 && argv)
    {
        if (argv[1] == std::string("-prof"))
        {
            eureka::PerfettoTracingConfig profiling_config{};

            perfettoTracing = std::make_unique<eureka::PerfettoTracing>(std::move(profiling_config));
            perfettoTracing->StartTracing();
        }
    }
#endif
    try
    {

        DEBUGGER_TRACE("createing app");

        PROFILE_PUSH_CATEGORIZED_RANGE("System Initialization", profiling::Color::Blue, profiling::PROFILING_CATEGORY_SYSTEM);
        App app;
        PROFILE_POP_RANGE(profiling::PROFILING_CATEGORY_SYSTEM);

        app.Run();

    }
    catch (const std::exception& err)
    {
        DEBUGGER_TRACE("{}", err.what());
    }

    return 0;
}