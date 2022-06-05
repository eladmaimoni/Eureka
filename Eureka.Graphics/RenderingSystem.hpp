#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GLFWRuntime.hpp>
#include "RenderTarget.hpp"

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



        void HandleResize(uint32_t width, uint32_t height);
        GLFWwindow* WindowHandle() { return _window.get(); }
    private:
        Instance& _instance;
        DeviceContext& _deviceContext;
        GLFWRuntime&   _glfw;

        GLFWWindowPtr                         _window; // TODO should remove
        std::unique_ptr<SwapChain>            _swapChain;
        std::shared_ptr<DepthColorRenderPass> _renderPass;
        std::vector<DepthColorRenderTarget>   _renderTargets;
        std::shared_ptr<vkr::Queue>           _presentationQueue;
        std::shared_ptr<vkr::Queue>           _graphicsQueue;

        // some object - maybe synchronized command buffer


        // this section should be a ring buffer of some sort
        uint32_t                              _maxFramesInFlight{};
        //uint32_t                              _currentFrame{ 0 };

        std::vector<vkr::CommandPool>         _mainGraphicsCommandPools;
        std::vector<vkr::CommandBuffer>       _mainGraphicsCommandBuffers;
        std::vector<vkr::Fence>               _renderingDoneFence;
        std::vector<vkr::Semaphore>           _renderingDoneSemaphore;




        void InitializeSwapChain(GLFWVulkanSurface& windowSurface);
        void InitializeCommandPoolsAndBuffers();
    };
}