#include "GLFWRuntime.hpp"
#include <debugger_trace.hpp>
#include <trigger_debugger_breakpoint.hpp>
#include <assert.hpp>

namespace eureka
{
    void glfw_error_callback(int error, const char* description)
    {
        DEBUGGER_TRACE("Glfw Error {}: {}", error, description);
        trigger_debugger_breakpoint();
    }

    GLFWRuntime::GLFWRuntime()
    {
        glfwSetErrorCallback(glfw_error_callback);
        [[maybe_unused]] auto result = glfwInit();
        assert(result == GLFW_TRUE);
    }

    GLFWRuntime::~GLFWRuntime()
    {
        glfwTerminate();
    }

    std::vector<const char*> GLFWRuntime::QueryRequiredVulkanExtentions() const
    {
        uint32_t extentions_count = 0;
        auto extentions = glfwGetRequiredInstanceExtensions(&extentions_count);
        return std::vector<const char*>(extentions, extentions + extentions_count);
    }

    GLFWVulkanSurface GLFWRuntime::CreateVulkanWindowSurface(int width, int height, VkInstance instance)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // no default rendering client, we'll hook vulkan later
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // resizing breaks the swapchain, we'll disable it for now
        auto window = glfwCreateWindow(width, height, "Eureka Engine", nullptr, nullptr);
        //int createdWidth = 0;
        //int createdHeight = 0;

        //glfwGetWindowSize(window, &createdWidth, &createdHeight);

        if (!window)
        {
            throw std::runtime_error("failed creating window");
        }
        VkSurfaceKHR c_style_surface;

        auto result = glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed creating window surface");
        }

        return GLFWVulkanSurface
        {
            .window = GLFWWindowPtr(window),
            .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
            .surface = c_style_surface
        };
            
    }

    

}