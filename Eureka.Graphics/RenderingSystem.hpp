#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"

#include <GLFWRuntime.hpp>
#include "GraphicsDefaults.hpp"

#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "ImGuiViewPass.hpp"

#include "IPass.hpp"

namespace eureka::graphics
{
    class SwapChain;
    

    struct RenderingSystemConfig
    {

    };

    //struct PendingSubmitFence
    //{
    //    vkr::Fence fence{ nullptr };
    //    bool       in_flight = false;
    //};

    class RenderingSystem
    {

    public:
        RenderingSystem(
            std::shared_ptr<vulkan::Device> device,
            vulkan::Queue graphicsQueue,
            vulkan::Queue copyQueue,
            std::shared_ptr<vulkan::FrameContext> frameContext,
            std::shared_ptr<ITargetPass> mainPass,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler   
        );

        ~RenderingSystem();

        void RunOne();
        void PollTasks();
        void Initialize();
        void HandleResize(uint32_t w, uint32_t h);
        void Deinitialize();
    private:
        std::shared_ptr<vulkan::Device>                            _device;          
        vulkan::Queue                                              _graphicsQueue;
        vulkan::Queue                                              _copyQueue;
        std::shared_ptr<vulkan::FrameContext>                      _frameContext;
        std::shared_ptr<SubmissionThreadExecutionContext>          _submissionThreadExecutionContext;
        std::shared_ptr<OneShotSubmissionHandler>                  _oneShotSubmissionHandler;
        sigslot::scoped_connection                                 _resizeConnection;
        std::chrono::high_resolution_clock::time_point             _lastFrameTime;
        std::shared_ptr<ITargetPass>                               _mainPass;
    };
}