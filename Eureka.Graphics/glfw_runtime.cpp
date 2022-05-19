#include "glfw_runtime.hpp"
#include <cassert>

namespace eureka
{

    GLFWRuntime::GLFWRuntime()
    {
        assert(glfwInit() == GLFW_TRUE);
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

}