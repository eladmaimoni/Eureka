#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <VkRuntime.hpp>
#include <vk_error_handling.hpp>

#include <glfw_runtime.hpp>

TEST_CASE("vk init", "[vulkan]")
{
    eureka::GLFWRuntime glfw;

    {
        eureka::VkRuntimeDesc runtime_desc{};
        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_instance_extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        runtime_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        eureka::VkRuntime runtime(std::move(runtime_desc));
  
        auto vkSurface = glfw.CreateVulkanSurface(runtime.Instance());
  
        
        eureka::VkDeviceContextDesc device_context_desc{};
        device_context_desc.presentation_surface = *vkSurface;
        device_context_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        eureka::VkDeviceContext device_context(runtime.Instance(), device_context_desc);
   


        //vk::CommandPoolCreateInfo graphicsThreadCommandPoolCreateInfo({}, device_context.Families().direct_graphics_family_index);
        //vk::raii::CommandPool     graphicsThreadCommandPool(*device_context.Device(), graphicsThreadCommandPoolCreateInfo);

        //vk::CommandBufferAllocateInfo graphicsCommandBufferAllocateInfo(*graphicsThreadCommandPool, vk::CommandBufferLevel::ePrimary, 1);
        //vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(*device_context.Device(), graphicsCommandBufferAllocateInfo).front());

        DEBUGGER_TRACE("hi");
  

    }

    DEBUGGER_TRACE("hi");

}