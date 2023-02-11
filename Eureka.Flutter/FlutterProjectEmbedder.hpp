#pragma once
#include "../Eureka.Windowing/Window.hpp"
#include "FlutterTaskRunners.hpp"
#include "FlutterVulkanCompositor.hpp"
#include <deque>
#include <filesystem>

#include "FlutterUtils.hpp"
#include <debugger_trace.hpp>

namespace eureka::flutter
{
    using namespace std::chrono_literals;

    const std::filesystem::path FLUTTER_EXAMPLE_DBG_PROJECT_OUT_PATH = "C:/workspace/ultrawis/build/";
    const std::filesystem::path FLUTTER_EXAMPLE_DBG_PROJECT_ASSETS_PATH =
        FLUTTER_EXAMPLE_DBG_PROJECT_OUT_PATH / "flutter_assets";
    //const std::filesystem::path FLUTTER_EXAMPLE_DBG_PROJECT_ICUDTL_PATH = FLUTTER_EXAMPLE_DBG_PROJECT_OUT_PATH / "icudtl.dat";
    const std::filesystem::path FLUTTER_EXAMPLE_DBG_PROJECT_ICUDTL_PATH =
        "C:/Libraries/flutter/bin/cache/artifacts/engine/windows-x64/icudtl.dat";

    const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH =
        "C:/workspace/ultrawis/build/windows/runner/Release/data";
    const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_ASSETS_PATH =
        FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "flutter_assets";
    const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_ICUDTL_PATH =
        FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "icudtl.dat";
    const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_AOT_ELF_PATH =
        FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "app.so";

    class FlutterVulkanCompositor;

    class FlutterProjectEmbedder
    {
        std::mutex                               _mtx;
        std::shared_ptr<FlutterVulkanCompositor> _compositor;
        std::shared_ptr<Window>                  _window;
        TaskRunner                               _platformTasksRunner;
        TaskRunner                               _renderTasksRunner;
        FlutterCustomTaskRunners                 _taskRunners;
        FlutterEngine                            _flutterEngine {nullptr};
        sigslot::scoped_connection               _winSize;
        sigslot::scoped_connection               _mouseButton;
        sigslot::scoped_connection               _cursorPos;
        std::deque<intptr_t>                     _pendingBatons;
        uint64_t                                 _lastPresentDone;
        uint64_t                                 _nextPresentTime;
        std::chrono::nanoseconds                 _frameDuration = 16667us;
        uint64_t                                 _frameDurationNs;
        double _cursorX{ 0.0 };
        double _cursorY{ 0.0 };
        bool _mouseDown = false;
    public:
        FlutterProjectEmbedder(std::shared_ptr<FlutterVulkanCompositor> compositor, std::shared_ptr<Window> window);

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
            auto self = static_cast<FlutterProjectEmbedder*>(userData);
            self->Vsync(baton);
        }

        void Vsync(intptr_t baton)
        {
            DEBUGGER_TRACE("Vsync");
            std::scoped_lock lk(_mtx);
            _pendingBatons.emplace_back(baton);
        }

        ~FlutterProjectEmbedder()
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

            _lastPresentDone = FlutterEngineGetCurrentTime();
            _nextPresentTime = _lastPresentDone + _frameDuration.count();
            _frameDurationNs = _frameDuration.count();
        }

        void DoTasks()
        {
            static constexpr uint64_t MILLI = std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count();
            _platformTasksRunner.PollReadyTasks();
            _renderTasksRunner.PollReadyTasks();

            auto now = FlutterEngineGetCurrentTime();

            if(now > _nextPresentTime || (_nextPresentTime - now) < MILLI)
            {
                // TODO atomic
                std::unique_lock lk(_mtx);

                if(!_pendingBatons.empty())
                {
                    auto oldestBaton = _pendingBatons.front();
                    _pendingBatons.pop_front();
                    lk.unlock();

                    _nextPresentTime = now + 1000000000 / 60;
                    FLUTTER_CHECK(FlutterEngineOnVsync(_flutterEngine, oldestBaton, now, _nextPresentTime));
                    //DEBUGGER_TRACE("FlutterEngineOnVsync {} {}", now, _nextPresentTime);
                }

                //FLUTTER_CHECK(FlutterEngineScheduleFrame(_flutterEngine));
            }
        }
    };

} // namespace eureka::flutter