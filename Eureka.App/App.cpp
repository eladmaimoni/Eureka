#include "App.hpp"
#include <debugger_trace.hpp>
#include <RenderingSystem.hpp>
#include <Window.hpp>
#include <profiling.hpp>
#include <system.hpp>
#include <AppTypes.hpp>
#include <json_io.hpp>
// TODO BUG BUG
#define MemoryBarrier __faststorefence 
#include <RemoteLiveSlamUI.hpp>
#include <RemoteLiveSlamClient.hpp>

namespace eureka
{
    static constexpr std::chrono::nanoseconds EXPECTERD_FRAME_DURATION = 16ms;
    static constexpr std::chrono::nanoseconds POLLING_TIME = EXPECTERD_FRAME_DURATION - 4ms;

    App::App()
    {


      
    }

    App::~App()
    {
    }

    void App::Run()
    {
        Initialize();
   
        auto lastFrameTimepoint = std::chrono::high_resolution_clock::now();
        auto pollingDeadline = lastFrameTimepoint + POLLING_TIME;

        while (!_window->ShouldClose())
        {
            PROFILE_CATEGORIZED_SCOPE("App::RunOne", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_SYSTEM);
            PollSystemEvents(pollingDeadline);
            _renderingSystem->RunOne();

            lastFrameTimepoint = std::chrono::high_resolution_clock::now();
            pollingDeadline = lastFrameTimepoint + POLLING_TIME;
        }
   


        Shutdown();
    }

    void App::Initialize()
    {
        os::set_system_timer_frequency(1ms);
        profiling::InitProfilingCategories();
        
        auto workingDir = std::filesystem::current_path();
        auto appMemoFile = workingDir / "app_memo.json";


        if (std::filesystem::exists(appMemoFile))
        {
            try
            {
                _memo = from_json_file<AppMemo>(appMemoFile);
            }
            catch (const std::exception& err)
            {
                CLOG("FAILED READING MEMO FILE {} err = {}", appMemoFile, err.what());
                std::filesystem::rename(appMemoFile, workingDir / "app_memo_that_failed_loading.json");

                _memo = AppMemo{};

            }

       
        }

        _container.Wire(_memo);

        _renderingSystem = _container.GetRenderingSystem();
        _window = _container.GetWindow();
        _remoteHandler = _container.GetRemoteHandler();
        _remoteUI = _container.GetRemoteUI();
       
    }

    void App::Shutdown()
    {
        _renderingSystem->Deinitialize();

        _remoteUI->UpdateMemo(_memo.liveslam);
        _window->UpdateMemo(_memo.window_config);

        auto workingDir = std::filesystem::current_path();
        auto appMemoFile = workingDir / "app_memo.json";

        to_json_file(_memo, appMemoFile);
    }

    void App::PollSystemEvents(std::chrono::high_resolution_clock::time_point deadline)
    {
        PROFILE_CATEGORIZED_SCOPE("PollSystemEvents", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_SYSTEM);

        auto now = std::chrono::high_resolution_clock::now();

        while (now < deadline)
        {
      
            auto leftover = deadline - now;

            if (leftover > 1ms)
            {
                _remoteHandler->PollCompletions(1ms);
            }
            else
            {
                _remoteHandler->PollCompletions();
            }

            _renderingSystem->PollTasks();
            _window->PollEvents();

            now = std::chrono::high_resolution_clock::now();
        }
 


    }

}