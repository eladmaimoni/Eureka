#pragma once
#include "../Eureka.Windowing/Window.hpp"
#include "TaskRunners.hpp"
#include "VulkanCompositor.hpp"
#include <deque>
#include <filesystem>

#include "FlutterUtils.hpp"
#include <debugger_trace.hpp>
#include <profiling.hpp>

namespace eureka::flutter
{
    using namespace std::chrono_literals;

    class VulkanCompositor;

    using UniqueAotDataPtr = std::unique_ptr<_FlutterEngineAOTData, FlutterEngineCollectAOTDataFnPtr>;

    struct EmbedderConfig
    {
        std::filesystem::path asset_dir;
        std::filesystem::path icudtl_path;
        std::filesystem::path aot_path;
    };

    struct FlutterNotifyVsyncRequest
    {
        FlutterNotifyVsyncRequest() = default;
        FlutterNotifyVsyncRequest(intptr_t baton, std::chrono::nanoseconds requestTimepoint) 
            : baton(baton), request_timepoint(requestTimepoint)
        {

        }
        intptr_t                 baton;
        std::chrono::nanoseconds request_timepoint;
    };

    class VulkanDesktopEmbedder
    {
        EmbedderConfig                    _config;
        UniqueAotDataPtr                  _aotData;
        std::mutex                        _mtx;
        std::shared_ptr<VulkanCompositor> _compositor;
        std::shared_ptr<Window>           _window;
        TaskRunner                        _combinedTaskRunner;
        FlutterCustomTaskRunners          _taskRunners;
        FlutterEngine                     _flutterEngine {nullptr};
        sigslot::scoped_connection        _winSize;
        sigslot::scoped_connection        _mouseButton;
        sigslot::scoped_connection        _cursorPos;
        std::deque<FlutterNotifyVsyncRequest>              _pendingVsyncNotifyRequests;

        std::chrono::nanoseconds          _frameDuration = 16667us;
        uint64_t                          _frameDurationNs;
        double                            _cursorX {0.0};
        double                            _cursorY {0.0};
        bool                              _mouseDown = false;

    public:
        VulkanDesktopEmbedder(EmbedderConfig                    config,
                       std::shared_ptr<VulkanCompositor> compositor,
                       std::shared_ptr<Window>           window);

        static void PlatformMessageStatic(const FlutterPlatformMessage* /* message*/, void* /* user data */)
        {
            DEBUGGER_TRACE("FlutterPlatformMessageCallback");
        }
        static void RootIsolateCreateStatic(void* /* user data */)
        {
            DEBUGGER_TRACE("RootIsolateStatic");
        }
        static void VsyncStatic(void* userData, intptr_t baton)
        {
            auto self = static_cast<VulkanDesktopEmbedder*>(userData);
            self->Vsync(baton);
        }

        void Vsync(intptr_t baton)
        {
            //DEBUGGER_TRACE("Vsync");
            std::scoped_lock lk(_mtx);
            _pendingVsyncNotifyRequests.emplace_back(baton, CurrentTimeNanoseconds());
        }

        ~VulkanDesktopEmbedder()
        {
            if(_flutterEngine)
            {
                FlutterEngineResult result = FlutterEngineShutdown(_flutterEngine);

                if(result != FlutterEngineResult::kSuccess)
                {
                    DEBUGGER_TRACE("failed to run flutter application");
                }
            }
        }

        void Run()
        {
            FLUTTER_CHECK(FlutterEngineRunInitialized(_flutterEngine));
            FlutterWindowMetricsEvent event = {};
            event.struct_size = sizeof(FlutterWindowMetricsEvent);
            event.width = _window->GetWidth();
            event.height = _window->GetHeight();
            event.pixel_ratio = static_cast<double>(event.width) / static_cast<double>(event.height);
            FLUTTER_CHECK(FlutterEngineSendWindowMetricsEvent(_flutterEngine, &event));


            _frameDurationNs = _frameDuration.count();
        }

        void Loop();
    };

} // namespace eureka::flutter