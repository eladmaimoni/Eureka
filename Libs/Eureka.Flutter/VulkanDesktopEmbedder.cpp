#include "VulkanDesktopEmbedder.hpp"

namespace eureka::flutter
{
    using namespace std::chrono_literals;

    VulkanDesktopEmbedder::VulkanDesktopEmbedder(EmbedderConfig                    config,
                                                 std::shared_ptr<VulkanCompositor> compositor,
                                                 std::shared_ptr<Window>           window) :

        _config(std::move(config)),
        _aotData(nullptr, FlutterEngineCollectAOTData),
        _compositor(std::move(compositor)),
        _window(std::move(window)),
        _combinedTaskRunner(std::this_thread::get_id())
    {

        _taskRunners.thread_priority_setter = [](FlutterThreadPriority) -> void { return; };
        _taskRunners.struct_size = sizeof(FlutterCustomTaskRunners);
        _taskRunners.platform_task_runner = &_combinedTaskRunner.GetDescription();
        _taskRunners.render_task_runner = &_combinedTaskRunner.GetDescription();

        auto        assets_path_str = _config.asset_dir.string();
        auto        icu_data_path_str = _config.icudtl_path.string();
        std::string aot_path_str;

        FlutterEngineAOTData aotData = nullptr;

        if(!_config.aot_path.empty())
        {
            aot_path_str = _config.aot_path.string();
            FlutterEngineAOTDataSource source = {};
            source.type = kFlutterEngineAOTDataSourceTypeElfPath;
            source.elf_path = aot_path_str.c_str();

            FLUTTER_CHECK(FlutterEngineCreateAOTData(&source, &aotData));
            _aotData.reset(aotData);
        }

        FlutterProjectArgs flutterProjectArgs {
            .struct_size = sizeof(FlutterProjectArgs),
            .assets_path = assets_path_str.c_str(),
            .icu_data_path = icu_data_path_str.c_str(),
            .platform_message_callback = nullptr, // PlatformMessageStatic,
            //.root_isolate_create_callback = RootIsolateCreateStatic,
            .vsync_callback = VsyncStatic,
            .custom_task_runners = &_taskRunners,
            .compositor = &_compositor->GetFlutterCompositor(),
            .aot_data = _aotData.get(),

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

    void VulkanDesktopEmbedder::Loop()
    {
        static constexpr uint64_t MILLI = std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count();
        while(!_window->ShouldClose())
        {
            _window->PollEvents();
            _combinedTaskRunner.RunReadyTasksFor(1ms);


            std::unique_lock lk(_mtx);
            while(!_pendingVsyncNotifyRequests.empty())
            {

                auto request = _pendingVsyncNotifyRequests.front();

                _pendingVsyncNotifyRequests.pop_front();
                lk.unlock();


                auto lastVsync = _compositor->GetLastPresntationTimepoint();
                auto nextVsync = lastVsync + FRAME_INTERVAL_60_FPS_NS;

                if (request.request_timepoint < lastVsync)
                {
                    // at the time of the request, last vsync did not yet occur. we should release flutter wait immediatly

                    FLUTTER_CHECK(FlutterEngineOnVsync(
                        _flutterEngine,
                        request.baton,
                        lastVsync.count(), // engine will not wait since this already happend
                        nextVsync.count()
                    ));
                }
                else
                {
                    // the request happened after last vsync, flutter should wait for next vsync
                    FLUTTER_CHECK(FlutterEngineOnVsync(
                        _flutterEngine,
                        request.baton,
                        nextVsync.count(), // engine will not wait to next vsync
                        (nextVsync + FRAME_INTERVAL_60_FPS_NS).count()
                    ));
                }

                PROFILE_CATEGORIZED_SCOPE("FlutterEngineOnVsync",
                                          eureka::profiling::Color::Green,
                                          eureka::profiling::PROFILING_CATEGORY_SYSTEM);

             

                lk.lock();
                //DEBUGGER_TRACE("FlutterEngineOnVsync {} {}", now, _nextPresentTime);
            }

                //FLUTTER_CHECK(FlutterEngineScheduleFrame(_flutterEngine));
            
        }
    }
} // namespace eureka::flutter