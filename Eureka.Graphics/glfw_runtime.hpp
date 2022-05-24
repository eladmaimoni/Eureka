#pragma once

#include <GLFW/glfw3.h>



namespace eureka
{
    class GLFWRuntime
    {
    private:
    public:
        GLFWwindow* _window;
    public:
        GLFWRuntime();

        ~GLFWRuntime();

        std::vector<const char*> QueryRequiredVulkanExtentions() const;
        vk::raii::SurfaceKHR CreateVulkanSurface(const vk::raii::Instance& instance);

    };
}