#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>



namespace eureka
{
    struct GLFWWindowDeleter
    {
        void operator()(GLFWwindow* window)
        {
            glfwDestroyWindow(window);
        }
    };

    using GLFWWindowPtr = std::unique_ptr<GLFWwindow, GLFWWindowDeleter>;


    struct GLFWVulkanSurface
    {
        GLFWWindowPtr        window;
        vk::Extent2D         size;
        vkr::SurfaceKHR surface;
    };

    class GLFWRuntime
    {

    public:
        GLFWRuntime();

        ~GLFWRuntime();

        std::vector<const char*> QueryRequiredVulkanExtentions() const;
        GLFWVulkanSurface CreateVulkanWindowSurface(
            int width, int height,
            const vkr::Instance& instance
        );

    };
}