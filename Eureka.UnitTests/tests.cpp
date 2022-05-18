#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <GLFW/glfw3.h>

#include "../Eureka.Graphics/vk_runtime.hpp"

TEST_CASE("vk init", "[vulkan]")
{
    uint32_t extentions_count = 0;

    assert(glfwInit() == GLFW_TRUE);

    auto extentions = glfwGetRequiredInstanceExtensions(&extentions_count);

    for (auto i = 0; i < extentions_count; ++i)
    {
        DEBUGGER_TRACE("extention required = {}", extentions[i]);
    }

    eureka::VkRuntimeDesc runtime_desc{};

    runtime_desc.required_instance_extentions = std::vector<const char*>(extentions, extentions + extentions_count);
    eureka::VkRuntime runtime(std::move(runtime_desc));

    DEBUGGER_TRACE("hi");

}