#pragma once
#include "GLFWRuntime.hpp"
#include "WindowTypes.hpp"
#include <sigslot/signal.hpp>
namespace eureka
{
;

    class Window
    {
        VkInstance                                  _instance{nullptr};
        GLFWVulkanSurface                           _windowSurface;
        sigslot::signal<uint32_t, uint32_t>         _resizeSignal;

    public:
        Window(
            GLFWRuntime& glfw,
            VkInstance instance,
            WindowConfig config
        );
        ~Window();
        bool ShouldClose();
        void PollEvents();
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        GLFWwindow* GetWindowHandle();
        VkSurfaceKHR GetSurface() const;
        template<typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }
        void SetPostion(const WindowPosition& position) const;
        WindowPosition GetPostion() const;

        int GetPostionX() const;
        int GetPostionY() const;
        
        void HandleResize(uint32_t width, uint32_t height);


        void UpdateMemo(WindowConfig& windowConfig) const;
    };

}

