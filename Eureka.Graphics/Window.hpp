#pragma once
#include "Instance.hpp"
#include "DeviceContext.hpp"
#include "GLFWRuntime.hpp"
#include "SwapChain.hpp"
#include <debugger_trace.hpp>


namespace eureka
{
    inline constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    inline constexpr int DEFAULT_WINDOW_HEIGHT = 768;

    class Window
    {
        DeviceContext&                              _deviceContext;
        Queue                                       _presentationQueue;
        Queue                                       _graphicsQueue;
        GLFWWindowPtr                               _window;
        std::shared_ptr<SwapChain>                  _swapChain;

    public:
        Window(
            GLFWRuntime& glfw,
            Instance& instance,
            DeviceContext& deviceContext,
            Queue graphicsQueue
        );
        
        bool ShouldClose();
        void PollEvents();


        GLFWwindow* WindowHandle() { return _window.get(); }

        std::shared_ptr<SwapChain> GetSwapChain() const
        {
            return _swapChain;
        }
        
        void InitializeSwapChain(GLFWVulkanSurface& windowSurface);
        void HandleResize(uint32_t width, uint32_t height);


    };

}

