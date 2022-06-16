#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GLFWRuntime.hpp>
#include "RenderTarget.hpp"
#include "Mesh.hpp"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"
#include "Camera.hpp"

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
            GLFWRuntime& glfw
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

        std::shared_ptr<RenderingUpdateQueue>          _updateQueue;

        GLFWWindowPtr                                        _window; // TODO should remove
        std::unique_ptr<SwapChain>                           _swapChain;
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        std::vector<DepthColorRenderTarget>                  _renderTargets;
        Queue                                                _presentationQueue;
        Queue                                                _graphicsQueue;
        Queue                                                _uploadQueue;

        //
        // upload
        //
        vkr::Semaphore                                       _uploadDoneSemaphore{ nullptr };
        vkr::Fence                                           _uploadDoneFence{ nullptr };
        CommandPool                                          _uploadPool; // TODO upload thread of some sort
        vkr::CommandBuffer                                   _uploadCommandBuffer{ nullptr };
        HostStageZoneBuffer                                  _stageZone;
        

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