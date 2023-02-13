#include "FlutterProjectEmbedder.hpp"

namespace eureka::flutter
{
    using namespace std::chrono_literals;



    //const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH =
    //    "C:/workspace";

    //"C:/workspace/ultrawis/build/windows/runner/Release/data";
    //const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_ASSETS_PATH =
    //    "flutter_ultwrawis_debug_with_release_engine";
    //FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "flutter_assets";
    //const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_ICUDTL_PATH =
    //    "C:/workspace/flutter_engine/Release/icudtl.dat";

    //FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "icudtl.dat";

    //const std::filesystem::path FLUTTER_EXAMPLE_RELEASE_PROJECT_AOT_ELF_PATH =
    //    FLUTTER_EXAMPLE_RELEASE_PROJECT_OUT_PATH / "app.so";
    Embedder::Embedder(EmbedderConfig                           config,
                       std::shared_ptr<FlutterVulkanCompositor> compositor,
                       std::shared_ptr<Window>                  window) :

        _config(std::move(config)),
        _compositor(std::move(compositor)),
        _window(std::move(window)),
        _combinedTaskRunner(std::this_thread::get_id())

    {

        _taskRunners.thread_priority_setter = [](FlutterThreadPriority) -> void { return; };
        _taskRunners.struct_size = sizeof(FlutterCustomTaskRunners);
        _taskRunners.platform_task_runner = &_combinedTaskRunner.GetDescription();
        _taskRunners.render_task_runner = &_combinedTaskRunner.GetDescription();

        auto assets_path_str = _config.asset_dir.string();
        auto icu_data_path_str = _config.icudtl_path.string();

        FlutterProjectArgs flutterProjectArgs {
            .struct_size = sizeof(FlutterProjectArgs),
            .assets_path = assets_path_str.c_str(),
            .icu_data_path = icu_data_path_str.c_str(),
            .platform_message_callback = nullptr, // PlatformMessageStatic,
            //.root_isolate_create_callback = RootIsolateCreateStatic,
            .vsync_callback = VsyncStatic,
            .custom_task_runners = &_taskRunners,
            .compositor = &_compositor->GetFlutterCompositor(),
            .aot_data = nullptr,

        };

        FlutterEngineResult result = FlutterEngineInitialize(FLUTTER_ENGINE_VERSION,
                                                             &_compositor->GetFlutterRendererConfig(),
                                                             &flutterProjectArgs,
                                                             this,
                                                             &_flutterEngine);

        if(result != FlutterEngineResult::kSuccess)
        {
            throw std::runtime_error("failed to initialize flutter application");
        }

        //_platformTasksRunner.SetEngineHandle(_flutterEngine);
        _combinedTaskRunner.SetEngineHandle(_flutterEngine);

        _winSize = _window->ConnectResizeSlot([this](uint32_t w, uint32_t h) {
            FlutterWindowMetricsEvent event = {};
            event.struct_size = sizeof(FlutterWindowMetricsEvent);
            event.width = w;
            event.height = h;
            event.pixel_ratio = static_cast<double>(w) / static_cast<double>(h);
            FLUTTER_CHECK(FlutterEngineSendWindowMetricsEvent(_flutterEngine, &event));
        });

        _mouseButton = _window->ConnectMouseButtonSlot([this](MouseButton button, MouseButtonState state) {
            if(button == MouseButton::eLeft)
            {
                FlutterPointerEvent event {};
                event.struct_size = sizeof(FlutterPointerEvent);
                event.phase =
                    (state == MouseButtonState::ePressed) ? FlutterPointerPhase::kDown : FlutterPointerPhase::kUp;
                event.timestamp = CurrentTimeMicroseconds();
                event.x = _cursorX;
                event.y = _cursorY;

                if(state == MouseButtonState::ePressed)
                {
                    _mouseDown = true;
                    event.phase = FlutterPointerPhase::kDown;
                }
                else
                {
                    _mouseDown = false;
                    event.phase = FlutterPointerPhase::kUp;
                }

                DEBUGGER_TRACE("mouse button phase {}", event.phase == FlutterPointerPhase::kDown ? "down" : "up");
                FLUTTER_CHECK(FlutterEngineSendPointerEvent(_flutterEngine, &event, 1));
            }
        }

        );

        _cursorPos = _window->ConnectCursorSlot([this](double x, double y) {
            _cursorX = x;
            _cursorY = y;

            FlutterPointerEvent event {};
            event.struct_size = sizeof(FlutterPointerEvent);
            event.phase = _mouseDown ? FlutterPointerPhase::kMove : FlutterPointerPhase::kHover;
            event.timestamp = CurrentTimeMicroseconds();
            event.x = x;
            event.y = y;

            FLUTTER_CHECK(FlutterEngineSendPointerEvent(_flutterEngine, &event, 1));
        });
    }

    void Embedder::Loop()
    {
        static constexpr uint64_t MILLI = std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count();
        while(!_window->ShouldClose())
        {
            _window->PollEvents();
            _combinedTaskRunner.RunReadyTasksFor(1ms);

            auto now = FlutterEngineGetCurrentTime();

            if(now > _nextPresentTime || (_nextPresentTime - now) < MILLI)
            {
                std::unique_lock lk(_mtx);

                if(!_pendingBatons.empty())
                {

                    auto oldestBaton = _pendingBatons.front();
                    _pendingBatons.pop_front();
                    lk.unlock();

                    _nextPresentTime = now + 1000000000 / 60;
                    PROFILE_CATEGORIZED_SCOPE("FlutterEngineOnVsync",
                                              eureka::profiling::Color::Green,
                                              eureka::profiling::PROFILING_CATEGORY_SYSTEM);
                    FLUTTER_CHECK(FlutterEngineOnVsync(_flutterEngine, oldestBaton, now, _nextPresentTime));
                    //DEBUGGER_TRACE("FlutterEngineOnVsync {} {}", now, _nextPresentTime);
                }

                //FLUTTER_CHECK(FlutterEngineScheduleFrame(_flutterEngine));
            }
        }
    }
} // namespace eureka::flutter