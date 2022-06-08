#include "GLFWRuntime.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{

    GLFWRuntime::GLFWRuntime()
    {
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

    GLFWVulkanSurface GLFWRuntime::CreateVulkanWindowSurface(int width, int height, const vkr::Instance& instance)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // no default rendering client, we'll hook vulkan later
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // resizing breaks the swapchain, we'll disable it for now
        auto window = glfwCreateWindow(width, height, "Eureka Engine", nullptr, nullptr);

        if (!window)
        {
            throw std::runtime_error("failed creating window");
        }
        VkSurfaceKHR c_style_surface;

        VK_CHECK(glfwCreateWindowSurface(*instance, window, nullptr, &c_style_surface));
        
        return GLFWVulkanSurface
        {
            .window = GLFWWindowPtr(window),
            .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
            .surface = vkr::SurfaceKHR(instance, c_style_surface)
        };
            
    }

    

}