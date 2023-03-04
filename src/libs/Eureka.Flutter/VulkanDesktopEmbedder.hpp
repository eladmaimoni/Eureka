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
        EmbedderConfig                            _config;
        UniqueAotDataPtr                          _aotData;
        std::mutex                                _mtx;
        std::shared_ptr<VulkanCompositor>         _compositor;
        std::shared_ptr<Window>                   _window;
        TaskRunner                                _combinedTaskRunner;
        FlutterCustomTaskRunners                  _taskRunners;
        FlutterEngine                             _flutterEngine {nullptr};
        std::deque<FlutterNotifyVsyncRequest>     _pendingVsyncNotifyRequests;


        //
        // input handling
        //
        sigslot::scoped_connection                _winSize;
        sigslot::scoped_connection                _mouseButton;
        sigslot::scoped_connection                _cursorPos;
        double                                    _cursorX {0.0};
        double                                    _cursorY {0.0};
        bool                                      _mouseDown = false;

    public:
        VulkanDesktopEmbedder(EmbedderConfig                    config,
                       std::shared_ptr<VulkanCompositor> compositor,
                       std::shared_ptr<Window>           window);


        ~VulkanDesktopEmbedder();

        void Run();
        void Loop();
    public:
        static void PlatformMessageStatic(const FlutterPlatformMessage* /* message*/, void* /* user data */);
        static void RootIsolateCreateStatic(void* /* user data */);
        static void EnqueueHandleNotifyOnNextVSyncRequestStatic(void* userData, intptr_t baton);


        void EnqueueHandleNotifyOnNextVSyncRequest(intptr_t baton);        
        void HandleNotifyOnNextVSyncRequest();
    };

} // namespace eureka::flutter