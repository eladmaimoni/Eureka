#include "FlutterProjectEmbedder.hpp"

namespace eureka::flutter
{

    FlutterProjectEmbedder::FlutterProjectEmbedder(std::shared_ptr<FlutterVulkanCompositor> compositor,
                                                   std::shared_ptr<Window>                  window) :
        _compositor(std::move(compositor)),
        _window(std::move(window)),
        _platformTasksRunner(std::this_thread::get_id()),
        _renderTasksRunner(std::this_thread::get_id())

    {

        _taskRunners.thread_priority_setter = [](FlutterThreadPriority) -> void { return; };
        _taskRunners.struct_size = sizeof(FlutterCustomTaskRunners);
        _taskRunners.platform_task_runner = &_platformTasksRunner.GetDescription();
        _taskRunners.render_task_runner = &_renderTasksRunner.GetDescription();

        auto assets_path_str = FLUTTER_EXAMPLE_DBG_PROJECT_ASSETS_PATH.string();
        auto icu_data_path_str = FLUTTER_EXAMPLE_DBG_PROJECT_ICUDTL_PATH.string();

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

        _platformTasksRunner.SetEngineHandle(_flutterEngine);
        _renderTasksRunner.SetEngineHandle(_flutterEngine);

        _winSize = _window->ConnectResizeSlot([this](uint32_t w, uint32_t h) {
            FlutterWindowMetricsEvent event = {};
            event.struct_size = sizeof(FlutterWindowMetricsEvent);
            event.width = w;
            event.height = h;
            event.pixel_ratio = static_cast<double>(w) / static_cast<double>(h);
            FLUTTER_CHECK(FlutterEngineSendWindowMetricsEvent(_flutterEngine, &event));
        });

        _mouseButton = _window->ConnectMouseButtonSlot(
            [this](MouseButton button, MouseButtonState state)
            {

                auto now = FlutterEngineGetCurrentTime();
                FlutterPointerEvent event{};
                event.struct_size = sizeof(FlutterPointerEvent);
                event.phase = (state == MouseButtonState::ePressed) ? FlutterPointerPhase::kDown : FlutterPointerPhase::kUp;
                event.timestamp = CurrentTimeMicroseconds();
                event.x = _cursorX;
                event.y = _cursorY;

                DEBUGGER_TRACE("mouse button phase {}", event.phase == FlutterPointerPhase::kDown ? "down" : "up");
                FLUTTER_CHECK(FlutterEngineSendPointerEvent(_flutterEngine, &event, 1));
            }

        );

        _cursorPos = _window->ConnectCursorSlot(
            [this](double x, double y)
            {
                _cursorX = x;
                _cursorY = y;
            }
        );
    }
} // namespace eureka::flutter