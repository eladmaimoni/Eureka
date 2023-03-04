#pragma once

//#include <windows.hpp> // https://stackoverflow.com/questions/3927810/how-to-prevent-macro-redefinition
//#ifndef VK_NO_PROTOTYPES 
//#define VK_NO_PROTOTYPES
//#endif
//#include <vulkan/vulkan.h>
#include <volk.h>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4005) // warning C4005 : 'APIENTRY' : macro redefinition
#include <GLFW/glfw3.h>
EUREKA_MSVC_WARNING_POP
#include <vector>
#include <memory>


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
        GLFWWindowPtr  window;
        VkExtent2D     size;
        VkSurfaceKHR   surface; // should be released by owner
    };

    class GLFWRuntime
    {
    public:
        GLFWRuntime();
        ~GLFWRuntime();

        std::vector<const char*> QueryRequiredVulkanExtentions() const;
        GLFWVulkanSurface CreateVulkanWindowSurface(
            int width, int height,
            VkInstance instance
        );

    };
}