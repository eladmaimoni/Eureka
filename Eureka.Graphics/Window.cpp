#include "Window.hpp"

namespace eureka
{


    Window::Window(GLFWRuntime& glfw, Instance& instance, DeviceContext& deviceContext, Queue graphicsQueue) :
        _deviceContext(deviceContext),
        _graphicsQueue(graphicsQueue)
    {
        auto windowSurface = glfw.CreateVulkanWindowSurface(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, instance.Get());

        _window = std::move(windowSurface.window);

        _presentationQueue = _deviceContext.CreatePresentQueue(*windowSurface.surface);

        InitializeSwapChain(windowSurface);


        glfwSetWindowUserPointer(_window.get(), this);
        glfwSetWindowSizeCallback(_window.get(), [](GLFWwindow* window, int width, int height)
            {
                auto userPtr = glfwGetWindowUserPointer(window);
                auto self = static_cast<Window*>(userPtr);
                self->HandleResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }
        );
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(_window.get());
    }
    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::InitializeSwapChain(GLFWVulkanSurface& windowSurface)
    {
        SwapChainTargetConfig swapChainDesc{};
        swapChainDesc.width = windowSurface.size.width;
        swapChainDesc.height = windowSurface.size.height;
        swapChainDesc.surface = std::move(windowSurface.surface);
        swapChainDesc.present_queue_family = _presentationQueue.Family();
        swapChainDesc.graphics_queue_family = _graphicsQueue.Family();

        _swapChain = std::make_shared<SwapChain>(_deviceContext, _presentationQueue, std::move(swapChainDesc));
    }

}

