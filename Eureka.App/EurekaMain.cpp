#include "App.hpp"
#include <debugger_trace.hpp>
#ifdef PERFETTO_TRACING
#include "perfetto_tracing_session.hpp"
#endif

using namespace eureka;

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

int main(int argc, char* argv[])
{
#ifdef PERFETTO_TRACING
    std::unique_ptr<eureka::PerfettoTracing> perfettoTracing;
    if (argc >= 2)
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
        App app;
  
        app.Run();


    }
    catch (const std::exception& err)
    {
        DEBUGGER_TRACE("{}", err.what());
    }

    return 0;
}