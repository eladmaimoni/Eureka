#pragma once
#include "GLFWRuntime.hpp"
#include "WindowTypes.hpp"
#include <sigslot/signal.hpp>

namespace eureka
{
    enum class MouseButton
    {
        eLeft,
        eRight,
        eMiddle,
        eBack,
        eForward,
    };
    enum class MouseButtonState
    {
        eRelease,
        ePressed,
    };

    class Window
    {
        VkInstance                                           _instance{nullptr};
        GLFWVulkanSurface                                    _windowSurface;
        sigslot::signal<uint32_t, uint32_t>                  _resizeSignal;
        sigslot::signal<MouseButton, MouseButtonState>       _mouseButtonSignal;
        sigslot::signal<double, double>                      _cursorSignal;
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

        void SetPostion(const WindowPosition& position) const;
        WindowPosition GetPostion() const;

        int GetPostionX() const;
        int GetPostionY() const;
        
   
        void Resize(uint32_t width, uint32_t height);
        void HandleResize(uint32_t width, uint32_t height);
        void UpdateMemo(WindowConfig& windowConfig) const;


        template<typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            //slot(GetWidth(), GetHeight());
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }
        template<typename Callable>
        sigslot::connection ConnectMouseButtonSlot(Callable&& slot)
        {
            return _mouseButtonSignal.connect(std::forward<Callable>(slot));
        }
        template<typename Callable>
        sigslot::connection ConnectCursorSlot(Callable&& slot)
        {
            return _cursorSignal.connect(std::forward<Callable>(slot));
        }
    private:
        static void MouseButtonStatic(GLFWwindow* window, int button, int action, int mods);
        static void CursorPosStatic(GLFWwindow* window, double x, double y);
        static void ScrollStatic(GLFWwindow* window, double xOffset, double yOffset);
        static void KeyStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    };

}

