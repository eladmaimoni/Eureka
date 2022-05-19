#include "glfw_runtime.hpp"
#include <cassert>
#include <stdexcept>
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

    vk::SurfaceKHR GLFWRuntime::CreateVulkanSurface(const vk::Instance& instance)
    {
        VkSurfaceKHR c_style_surface;
        VK_CHECK(glfwCreateWindowSurface(instance, _window, nullptr, &c_style_surface));
        return c_style_surface;
    }

}