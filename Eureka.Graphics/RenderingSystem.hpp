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

namespace eureka
{
    class SwapChain;
    
    


    struct RenderingSystemConfig
    {

    };

    class RenderingSystem
    {
    public:
        RenderingSystem(
            Instance& instance,
            DeviceContext& deviceContext,
            GLFWRuntime& glfw,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            Queue graphicsQueue,
            Queue copyQueue
        );

        ~RenderingSystem();

        void RunOne();
        void Initialize();

        void HandleSwapChainResize();

        void Deinitialize();
        
        void HandleResize(uint32_t width, uint32_t height);
        GLFWwindow* WindowHandle() { return _window.get(); }
    private:
        Instance& _instance;
        DeviceContext& _deviceContext;
        GLFWRuntime&   _glfw;

        std::shared_ptr<SubmissionThreadExecutionContext>    _submissionThreadExecutionContext;
        std::shared_ptr<RenderingUpdateQueue>          _updateQueue;

        GLFWWindowPtr                                        _window; // TODO should remove
        std::unique_ptr<SwapChain>                           _swapChain;
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        std::vector<DepthColorRenderTarget>                  _renderTargets;
        Queue                                                _presentationQueue;
        Queue                                                _graphicsQueue;
        Queue                                                _copyQueue;

        //
        // upload
        //
        vkr::Semaphore                                       _uploadDoneSemaphore{ nullptr };
        vkr::Fence                                           _uploadDoneFence{ nullptr };
        CommandPool                                          _uploadPool; // TODO upload thread of some sort
        vkr::CommandBuffer                                   _uploadCommandBuffer{ nullptr };
        HostWriteCombinedBuffer                              _stageZone;
        

        DescriptorPool                                       _descPool;
        DescriptorSet                                        _constantBufferSet;

        // triangle stuff
        PerspectiveCamera                                    _camera;
        VertexAndIndexTransferableDeviceBuffer               _triangle;

        std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> _perFrameDescriptorSet;
        ColoredVertexMeshPipeline                            _coloredVertexPipeline;


        // this section should be a ring buffer of some sort
        uint32_t                                       _maxFramesInFlight{};

        std::vector<FrameCommands>                     _frameCommandBuffer;

        std::chrono::high_resolution_clock::time_point _lastFrameTime;
     

        void InitializeSwapChain(GLFWVulkanSurface& windowSurface);
        void InitializeCommandPoolsAndBuffers();
    };
}