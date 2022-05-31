#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <VkRuntime.hpp>
#include <vk_error_handling.hpp>
#include <VkHelpers.hpp>
#include <SwapChainTarget.hpp>
#include <glfw_runtime.hpp>
#include <ShadersCache.hpp>

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
        device_context_desc.presentation_surface = *presentationSurface.surface;
        device_context_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        device_context_desc.required_extentions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        eureka::VkDeviceContext device_context(runtime.Instance(), device_context_desc);
   

        eureka::SwapChainTargetDesc swapChainDesc{};
        swapChainDesc.width = presentationSurface.size.width;
        swapChainDesc.height = presentationSurface.size.height;
        swapChainDesc.surface = std::move(presentationSurface.surface);
        swapChainDesc.physical_device = device_context.PhysicalDevice().get();
        swapChainDesc.logical_device = device_context.Device().get();
        swapChainDesc.present_queue_family = device_context.Families().present_family_index;
        swapChainDesc.graphics_queue_family = device_context.Families().direct_graphics_family_index;


        eureka::SwapChainTarget target(runtime, std::move(swapChainDesc));

        vk::CommandPoolCreateInfo graphicsThreadCommandPoolCreateInfo{ .flags = {}, .queueFamilyIndex = device_context.Families().direct_graphics_family_index };
        vk::raii::CommandPool     graphicsThreadCommandPool(*device_context.Device(), graphicsThreadCommandPoolCreateInfo);

        vk::CommandBufferAllocateInfo graphicsCommandBufferAllocateInfo{ .commandPool = *graphicsThreadCommandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
        vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(*device_context.Device(), graphicsCommandBufferAllocateInfo).front());


        eureka::ShaderCache shaderCache(device_context.Device());

        DEBUGGER_TRACE("hi");
  

    }

    DEBUGGER_TRACE("hi");

}

