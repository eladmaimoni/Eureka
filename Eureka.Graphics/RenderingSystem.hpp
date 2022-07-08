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
        std::shared_ptr<SwapChain>                  _swapChain;
    public:
        RenderingSystem(
            DeviceContext& deviceContext,
            std::shared_ptr<SwapChain> swapChain,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            std::shared_ptr<OneShotCopySubmissionHandler> oneShotCopySubmissionHandler,
            std::shared_ptr<MTDescriptorAllocator>                   descPool,
            Queue graphicsQueue,
            Queue copyQueue
        );

        ~RenderingSystem();

        void RunOne();


        void Initialize();
        void HandleSwapChainResize();
        void Deinitialize();
        

        // temporary until proper hierarchical init
        // TODO REMOVE init from outside
        std::shared_ptr<PipelineCache> GetPipelineCache()
        {
            return _pipelineCache;
        }
    private:
        DeviceContext&                                             _deviceContext;          
        Queue                                                      _graphicsQueue;
        Queue                                                      _copyQueue;

        std::shared_ptr<SubmissionThreadExecutionContext>          _submissionThreadExecutionContext;
        std::shared_ptr<OneShotCopySubmissionHandler>              _oneShotCopySubmissionHandler;

        std::shared_ptr<DepthColorRenderPass>                      _renderPass;
        std::vector<DepthColorRenderTarget>                        _renderTargets;
        std::vector<FrameCommands>                                 _frameCommandBuffer;

        std::shared_ptr<MTDescriptorAllocator>                            _descPool;
        std::shared_ptr<PipelineCache>                             _pipelineCache;
          

        HostWriteCombinedBuffer                                    _stageZone;                                                                  
        DescriptorSet                                              _constantBufferSet;
        VertexAndIndexTransferableDeviceBuffer                     _triangle;
                                                                   
        // triangle stuff                                          
        PerspectiveCamera                                          _camera;


        std::shared_ptr<ColoredVertexMeshPipeline>                 _coloredVertexPipeline;


        // this section should be a ring buffer of some sort
        uint32_t                                                   _maxFramesInFlight{};

        std::chrono::high_resolution_clock::time_point             _lastFrameTime;
     

        void InitializeCommandPoolsAndBuffers();


        void WaitForFrame(vk::Fence currentFrameFence);

        void SubmitFrame(
            const vkr::CommandBuffer& renderingCommandBuffer,
            vk::Semaphore imageReadySemaphore,
            vk::Semaphore renderingDoneSemaphore,
            vk::Fence renderingDoneFence
        );

        void RecordMainRenderPass(uint32_t currentFrame, vkr::CommandBuffer& renderingCommandBuffer);
        sigslot::scoped_connection _resizeConnection;
    };
}