
#include "perfetto_tracing_session.hpp"
#include "profiling_categories.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <compiler.hpp>
#include <debugger_trace.hpp>

namespace trace
{
    enum TraceType { kInProcess, kSystem };
    void InitializeTrace(TraceType trace_type = TraceType::kSystem);

    // Those methods are needed when using in process tracing, for example in a unit test mode.
    // Otherwise, the start is done from the command line, and flushing takes place in the SAVE_PROFILER_REPORT macro.
    std::unique_ptr<perfetto::TracingSession> StartTracing(uint32_t buff_size = 1024);

    void StopTracing(std::unique_ptr<perfetto::TracingSession> tracing_session, const std::string& trace_file = "slam_example.pftrace");
} // namespace trace



namespace trace 
{
    void InitializeTrace(TraceType trace_type)
    {
        perfetto::TracingInitArgs args;
        // The backends determine where trace events are recorded.
        // The in-process tracing includes in-app events.
        // The system-wide tracing service lets us see our app's events in context
        // with system profiling information.
        args.backends = trace_type == TraceType::kInProcess
                            ? perfetto::BackendType::kInProcessBackend
                            : perfetto::BackendType::kSystemBackend;
        PERFETTO_LOG("Initialized perfetto tracing");
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();
    }

    std::unique_ptr<perfetto::TracingSession> StartTracing(uint32_t buff_size)
    {
        // The trace config defines which types of data sources are enabled for
        // recording. 'track_event' are for in app only events.

        perfetto::protos::gen::TrackEventConfig track_event_cfg;
        //track_event_cfg.
        //track_event_cfg.add_disabled_categories("*");
        //track_event_cfg.add_enabled_categories("rendering");

        perfetto::TraceConfig cfg;
        cfg.add_buffers()->set_size_kb((uint32_t)buff_size);
        auto* ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name("track_event");

        auto tracing_session = perfetto::Tracing::NewTrace();
        tracing_session->Setup(cfg);
        tracing_session->StartBlocking();
        return tracing_session;
    }

    void StopTracing(std::unique_ptr<perfetto::TracingSession> tracing_session, const std::string& trace_file) 
    {
        // Make sure the last event is closed .
        perfetto::TrackEvent::Flush();

        // Stop tracing and read the trace data.
        tracing_session->StopBlocking();
        std::vector<char> trace_data(tracing_session->ReadTraceBlocking());

        // Write the result into a file.
        // Note: To save memory with longer traces, you can tell Perfetto to write
        // directly into a file by passing a file descriptor into Setup() above.
        std::ofstream output;
        output.open(trace_file, std::ios::out | std::ios::binary);
        output.write(&trace_data[0], static_cast<std::streamsize>(trace_data.size()));
        output.close();
        PERFETTO_LOG("Tracing stopped, wrote to %s", trace_file.c_str());
    }
}  // namespace trace

namespace eureka
{
    void SetPerfettoThreadName(std::string_view thread_name)
    {
        // https://github.com/google/perfetto/issues/351#event-7376713200
        if (perfetto::Tracing::IsInitialized())
        {
            auto track = perfetto::ThreadTrack::Current();
            auto desc = track.Serialize();
            desc.mutable_thread()->set_thread_name(thread_name.data());
            perfetto::TrackEvent::SetTrackDescriptor(track, std::move(desc));
        }
    }

    PerfettoTracing::PerfettoTracing(PerfettoTracingConfig config) : _config(std::move(config))
    {
        perfetto::TracingInitArgs args;
        // The backends determine where trace events are recorded.
        // The in-process tracing includes in-app events.
        // The system-wide tracing service lets us see our app's events in context
        // with system profiling information.
        args.backends |= perfetto::kInProcessBackend;
#ifndef _WIN32
        if (_config.do_system_profiling)
        {
            args.backends |= perfetto::kSystemBackend;
        }
#endif
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();

        PERFETTO_LOG("Initialized perfetto tracing");
        }

    EUREKA_MSVC_WARNING_PUSH
    EUREKA_MSVC_WARNING_DISABLE(4996)

    void PerfettoTracing::StartTracing()
    {
        if (_activeTracing)
        {
            PERFETTO_LOG("tracing already active");
            return;
        }

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream str;
        str << std::put_time(&tm, "-%Y-%m-%d--%H-%M-%S");

        _currentTraceFileName = _config.output_dir / (_config.trace_file_prefix + str.str() + ".pftrace");

        _activeTracing = trace::StartTracing(_config.tracing_file_size);
    }

    EUREKA_MSVC_WARNING_POP

    PerfettoTracing::~PerfettoTracing()
    {
        try
        {
            StopTracing();
        }
        catch (const std::exception& err)
        {
            DEBUGGER_TRACE("{}", err.what());
        }
    }


    void PerfettoTracing::StopTracing()
    {
        if (_activeTracing)
        {
            trace::StopTracing(std::move(_activeTracing), _currentTraceFileName.string());
        }
    }

}