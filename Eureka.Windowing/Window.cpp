#include "Window.hpp"
#include <debugger_trace.hpp>


namespace eureka
{
    Window::Window(GLFWRuntime& glfw, VkInstance instance, WindowConfig config)
        :
        _instance(instance),
        _windowSurface(glfw.CreateVulkanWindowSurface(config.width, config.height, _instance))
    {


        SetPostion(config.position);
 

        glfwSetWindowUserPointer(_windowSurface.window.get(), this);

        glfwSetWindowSizeCallback(_windowSurface.window.get(), [](GLFWwindow* window, int width, int height)
            {
                try
                {
                    auto userPtr = glfwGetWindowUserPointer(window);
                    auto self = static_cast<Window*>(userPtr);
                    self->HandleResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
                }
                catch (const std::exception& err)
                {
                    DEBUGGER_TRACE("err = {}", err.what());
                }

            }
        );

        //glfwSetCursorEnterCallback(_window.get(), ImGui_ImplGlfw_CursorEnterCallback);
        glfwSetCursorPosCallback(_windowSurface.window.get(), [](GLFWwindow* /*window*/, double /*x*/, double /*y*/)
            {
                //DEBUGGER_TRACE("cursor position = ({},{})", x, y);
            });
        glfwSetMouseButtonCallback(_windowSurface.window.get(), [](GLFWwindow* /*window*/, int /*button*/, int /*action*/, int /*mods*/)
        {
                //DEBUGGER_TRACE("mouse button = {} action = {} mods = {})", button, action, mods);
        }       
        );
        // ImGui_ImplGlfw_MouseButtonCallback

        glfwSetScrollCallback(_windowSurface.window.get(), [](GLFWwindow* /*window*/, double /*xoffset*/, double /*yoffset*/)
            {
                //DEBUGGER_TRACE("scroll offset position = ({},{})", xoffset, yoffset);
            });
        //glfwSetKeyCallback(_window.get(), ImGui_ImplGlfw_KeyCallback);
        //glfwSetCharCallback(_window.get(), ImGui_ImplGlfw_CharCallback);


        //_presentationQueue = _deviceContext.CreatePresentQueue(*windowSurface.surface);

        //InitializeSwapChain(windowSurface);

   
    }

    Window::~Window()
    {
        if (_windowSurface.surface && _instance)
        {
            vkDestroySurfaceKHR(_instance, _windowSurface.surface, nullptr);
        }
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(_windowSurface.window.get());

        
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    uint32_t Window::GetWidth() const
    {
        return _windowSurface.size.width;
    }

    uint32_t Window::GetHeight() const
    {
        return _windowSurface.size.height;
    }

    GLFWwindow* Window::GetWindowHandle()
    {
        return _windowSurface.window.get();
    }

    VkSurfaceKHR Window::GetSurface() const
    {
        return _windowSurface.surface;
    }

    void Window::SetPostion(const WindowPosition& position) const
    {
        glfwSetWindowPos(_windowSurface.window.get(), position.x, position.y);
    }

    WindowPosition Window::GetPostion() const
    {
        int _valX = 0;
        int _valY = 0;
        glfwGetWindowPos(_windowSurface.window.get(), &_valX, &_valY);
        return { _valX, _valY };
    }


    int Window::GetPostionX() const
    {
        return GetPostion().x;
    }
    int Window::GetPostionY() const
    {
        return GetPostion().y;
    }

    void Window::Resize(uint32_t width, uint32_t height)
    {
        glfwSetWindowSize(_windowSurface.window.get(), static_cast<int>(width), static_cast<int>(height));
        HandleResize(width, height);
    }

    void Window::HandleResize(uint32_t width, uint32_t height)
    {
        _windowSurface.size.width = width;
        _windowSurface.size.height = height;
        _resizeSignal(width, height);

        //DEBUGGER_TRACE("HandleResize({},{})", width, height);
    }

    void Window::UpdateMemo(WindowConfig& windowConfig) const
    {
        windowConfig.position = GetPostion();
        windowConfig.width = GetWidth();
        windowConfig.height = GetHeight();
    }

}

