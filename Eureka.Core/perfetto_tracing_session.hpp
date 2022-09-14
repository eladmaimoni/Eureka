#pragma once


#include <memory>
#include <filesystem>

namespace perfetto
{
    class TracingSession;
}

namespace eureka
{
    struct PerfettoTracingConfig
    {
        bool do_system_profiling = false;
        uint32_t tracing_file_size = 1024 * 1024;
        std::string trace_file_prefix = "eureka_profiling";
        std::filesystem::path output_dir = std::filesystem::current_path();
    };

    class PerfettoTracing
    {
        PerfettoTracingConfig _config;
        std::unique_ptr<perfetto::TracingSession> _activeTracing;
        std::filesystem::path _currentTraceFileName;
    public:
        PerfettoTracing(PerfettoTracingConfig config);
        ~PerfettoTracing();
        void StartTracing();
        void StopTracing();
    };
}