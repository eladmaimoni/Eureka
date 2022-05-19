#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <string_view>


namespace eureka
{
    class GLFWRuntime
    {
    private:
        GLFWwindow* _window;
    public:
        GLFWRuntime();

        ~GLFWRuntime();

        std::vector<const char*> QueryRequiredVulkanExtentions() const;
    };
}