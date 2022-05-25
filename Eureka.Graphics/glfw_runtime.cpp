#include "glfw_runtime.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{

    GLFWRuntime::GLFWRuntime()
    {
        assert(glfwInit() == GLFW_TRUE);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // no default rendering client, we'll hook vulkan later
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // resizing breaks the swapchain, we'll disable it for now

         
        _window = glfwCreateWindow(1200, 800, "Eureka Engine", nullptr, nullptr);
        
        if (!_window)
        {
            throw std::runtime_error("failed creating window");
        }
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

    GLFWVulkanSurface GLFWRuntime::CreateVulkanSurface(const vk::raii::Instance& instance)
    {
        VkSurfaceKHR c_style_surface;
        int win_width, win_height;
        VK_CHECK(glfwCreateWindowSurface(*instance, _window, nullptr, &c_style_surface));
        glfwGetWindowSize(_window, &win_width, &win_height);
        return GLFWVulkanSurface
        {
            .size = {static_cast<uint32_t>(win_width), static_cast<uint32_t>(win_height)},
            .surface = vk::raii::SurfaceKHR(instance, c_style_surface)
        };
            
    }

    

}