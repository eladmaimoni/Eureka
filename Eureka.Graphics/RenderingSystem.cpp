#include "RenderingSystem.hpp"
#include "SwapChainTarget.hpp"
#include <debugger_trace.hpp>

namespace eureka
{
    inline constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    inline constexpr int DEFAULT_WINDOW_HEIGHT = 768;
    inline constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };


    RenderingSystem::RenderingSystem(
        Instance& instance,
        DeviceContext& deviceContext,
        GLFWRuntime& glfw
    )
        :
        _glfw(glfw),
        _instance(instance),
        _deviceContext(deviceContext)
    {

    }

    RenderingSystem::~RenderingSystem()
    {

    }

    void RenderingSystem::RunOne()
    {
        _mainGraphicsCommandPool.reset();
    }

    void RenderingSystem::Initialize()
    {
        //
        // this section should be moved to some sort of window class
        //

        auto windowSurface = _glfw.CreateVulkanWindowSurface(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, _instance.Get());
        _deviceContext.InitializePresentationQueueFromExistingQueues(*windowSurface.surface);

        _window = std::move(windowSurface.window);

        glfwSetWindowUserPointer(_window.get(), this);
        glfwSetWindowSizeCallback(_window.get(), [](GLFWwindow* window, int width, int height)
            {
                auto userPtr = glfwGetWindowUserPointer(window);
                auto self = static_cast<RenderingSystem*>(userPtr);
                self->HandleResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }
        );



        _presentationQueue = _deviceContext.PresentQueue();
        _graphicsQueue = _deviceContext.GraphicsQueue().front();

        InitializeSwapChain(windowSurface);
        InitializeCommandPoolsAndBuffers();


        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        vk::FenceCreateInfo fenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _inFlightFences.emplace_back(_deviceContext.LogicalDevice()->createFence(fenceCreateInfo));
            _imageAvailableSemaphore.emplace_back(_deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo));
            _renderFinishedSemaphore.emplace_back(_deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo));
        }

        

        ////_deviceContext.PhysicalDevice()->getFormatProperties();

    }

    void RenderingSystem::InitializeSwapChain(GLFWVulkanSurface& windowSurface)
    {
        SwapChainTargetDesc swapChainDesc{};
        swapChainDesc.width = windowSurface.size.width;
        swapChainDesc.height = windowSurface.size.height;
        swapChainDesc.surface = std::move(windowSurface.surface);

        swapChainDesc.present_queue_family = _deviceContext.Families().present_family_index;
        swapChainDesc.graphics_queue_family = _deviceContext.Families().direct_graphics_family_index;

        _primaryTarget = std::make_unique<SwapChainTarget>(_deviceContext, std::move(swapChainDesc));
    }

    void RenderingSystem::InitializeCommandPoolsAndBuffers()
    {
        vk::CommandPoolCreateInfo graphicsThreadCommandPoolCreateInfo
        {
            .flags = {}, // no flags, means we can only reset the pool and not the command buffers
            .queueFamilyIndex = _deviceContext.Families().direct_graphics_family_index
        };

        _mainGraphicsCommandPool = vkr::CommandPool(
            *_deviceContext.Device(),
            vk::CommandPoolCreateInfo
            {
                .flags = {}, // no flags, means we can only reset the pool and not the command buffers
                .queueFamilyIndex = _deviceContext.Families().direct_graphics_family_index
            }
        );

        vk::CommandBufferAllocateInfo graphicsCommandBufferAllocateInfo
        {
            .commandPool = *_mainGraphicsCommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = _primaryTarget->ImageCount()
        };

        _mainGraphicsCommandBuffers = static_cast<std::vector<vkr::CommandBuffer>&&>(vkr::CommandBuffers(*_deviceContext.Device(), graphicsCommandBufferAllocateInfo));
    
        _mainCommandBuffersFence = _deviceContext.LogicalDevice()->createFence(
            vk::FenceCreateInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled
            }
        );
            
    }

    void RenderingSystem::HandleResize(uint32_t width, uint32_t height)
    {
        DEBUGGER_TRACE("HandleResize({},{})", width, height);
        _primaryTarget->Resize(width, height);
    }

}