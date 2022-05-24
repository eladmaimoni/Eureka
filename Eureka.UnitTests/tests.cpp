#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <VkRuntime.hpp>
#include <vk_error_handling.hpp>
#include <SwapChainTarget.hpp>
#include <glfw_runtime.hpp>

TEST_CASE("vk init", "[vulkan]")
{
    eureka::GLFWRuntime glfw;

    {
        eureka::GPURuntimeDesc runtime_desc{};
        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_instance_extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        runtime_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        eureka::GPURuntime runtime(std::move(runtime_desc));
  
        auto presentationSurface = glfw.CreateVulkanSurface(runtime.Instance());
  
        
        eureka::VkDeviceContextDesc device_context_desc{};
        device_context_desc.presentation_surface = *presentationSurface;
        device_context_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        eureka::VkDeviceContext device_context(runtime.Instance(), device_context_desc);
   

        eureka::SwapChainTargetDesc swapChainDesc{};
        swapChainDesc.surface = *presentationSurface;
        swapChainDesc.physical_device = device_context.PhysicalDevice().get();


        eureka::SwapChainTarget target(runtime, swapChainDesc);

        vk::CommandPoolCreateInfo graphicsThreadCommandPoolCreateInfo{ .flags = {}, .queueFamilyIndex = device_context.Families().direct_graphics_family_index };
        vk::raii::CommandPool     graphicsThreadCommandPool(*device_context.Device(), graphicsThreadCommandPoolCreateInfo);

        vk::CommandBufferAllocateInfo graphicsCommandBufferAllocateInfo{ .commandPool = *graphicsThreadCommandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
        vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(*device_context.Device(), graphicsCommandBufferAllocateInfo).front());

        DEBUGGER_TRACE("hi");
  

    }

    DEBUGGER_TRACE("hi");

}