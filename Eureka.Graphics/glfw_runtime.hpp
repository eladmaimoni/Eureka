#pragma once

#include <GLFW/glfw3.h>



namespace eureka
{
    struct GLFWVulkanSurface
    {
        vk::Extent2D size;
        vk::raii::SurfaceKHR surface;
    };

    class GLFWRuntime
    {
    private:
    public:
        GLFWwindow* _window;
    public:
        GLFWRuntime();

        ~GLFWRuntime();

        std::vector<const char*> QueryRequiredVulkanExtentions() const;
        GLFWVulkanSurface CreateVulkanSurface(const vk::raii::Instance& instance);

    };
}