#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <glfw_runtime.hpp>
#include <vk_runtime.hpp>

TEST_CASE("vk init", "[vulkan]")
{
    eureka::GLFWRuntime glfw;

    {
        eureka::VkRuntimeDesc runtime_desc{};
        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_layers.emplace_back(eureka::VALIDATION_LAYER_NAME);
        eureka::VkRuntime runtime(std::move(runtime_desc));

    }

    DEBUGGER_TRACE("hi");

}