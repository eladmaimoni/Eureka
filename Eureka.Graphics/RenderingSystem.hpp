#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GLFWRuntime.hpp>

namespace eureka
{
    class SwapChainTarget;
    
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

        GLFWWindowPtr                    _window; // TODO should remove
        std::unique_ptr<SwapChainTarget> _primaryTarget;

        std::shared_ptr<vkr::Queue> _presentationQueue;
        std::shared_ptr<vkr::Queue> _graphicsQueue;

        // some object - maybe synchronized command buffer
        vkr::CommandPool                _mainGraphicsCommandPool{nullptr};
        std::vector<vkr::CommandBuffer> _mainGraphicsCommandBuffers;
        vkr::Fence                      _mainCommandBuffersFence{nullptr}; // should be a vector?

        std::vector<vkr::Fence>         _inFlightFences;
        std::vector<vkr::Semaphore>     _imageAvailableSemaphore;
        std::vector<vkr::Semaphore>     _renderFinishedSemaphore;




        void InitializeSwapChain(GLFWVulkanSurface& windowSurface);
        void InitializeCommandPoolsAndBuffers();
    };
}