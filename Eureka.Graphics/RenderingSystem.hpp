#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GLFWRuntime.hpp>
#include "GraphicsDefaults.hpp"
#include "RenderTarget.hpp"
#include "Mesh.hpp"
#include "Commands.hpp"
#include "Pipeline.hpp"
#include "Camera.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "ImGuiViewPass.hpp"
#include "FrameContext.hpp"
#include "IPass.hpp"

namespace eureka
{
    class SwapChain;
    

    struct RenderingSystemConfig
    {

    };

    struct PendingSubmitFence
    {
        vkr::Fence fence{ nullptr };
        bool       in_flight = false;
    };

    class RenderingSystem
    {

    public:
        RenderingSystem(
            DeviceContext& deviceContext,
            Queue graphicsQueue,
            Queue copyQueue,
            std::shared_ptr<FrameContext> frameContext,
            std::shared_ptr<ITargetPass> mainPass,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler   
        );

        ~RenderingSystem();

        void RunOne();

        void Initialize();
        void HandleResize(uint32_t w, uint32_t h);
        void Deinitialize();
        
    private:
        DeviceContext&                                             _deviceContext;          
        Queue                                                      _graphicsQueue;
        Queue                                                      _copyQueue;
        std::shared_ptr<FrameContext>                              _frameContext;
        std::shared_ptr<SubmissionThreadExecutionContext>          _submissionThreadExecutionContext;
        std::shared_ptr<OneShotSubmissionHandler>                  _oneShotSubmissionHandler;
        sigslot::scoped_connection                                 _resizeConnection;
        std::chrono::high_resolution_clock::time_point             _lastFrameTime;
        std::shared_ptr<ITargetPass>                               _mainPass;

       
  
    };
}