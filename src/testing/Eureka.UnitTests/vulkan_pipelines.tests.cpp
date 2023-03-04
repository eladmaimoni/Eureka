#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Pipeline.hpp"
#include "../Eureka.Vulkan/PipelinePresets.hpp"
#include "../Eureka.Vulkan/RenderTarget.hpp"
#include "../Eureka.Vulkan/ResourceAllocator.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include "../Eureka.Windowing/Window.hpp"
#include <boost/hana/for_each.hpp>
#include <catch.hpp>
#include <debugger_trace.hpp>
#include <sigslot/signal.hpp>

namespace vk = eureka::vulkan;

TEST_CASE("imgui pipeline creation", "[vulkan]")
{
    eureka::GLFWRuntime glfw;
    auto                instance = vk::MakeDefaultInstance();
    auto                window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig {});
    auto                device = vk::MakeDefaultDevice(instance, window->GetSurface());
    auto                allocator = std::make_shared<vk::ResourceAllocator>(instance, device);

    auto graphicsQueue = device->GetGraphicsQueue();
    auto presntationQueue = device->GetPresentQueue();

    vk::SwapChain swapChain(window, device, presntationQueue, graphicsQueue);

    vk::DepthColorRenderPassConfig depthColorConfig {.color_output_format = swapChain.ImageFormat(),
                                                     .depth_output_format = vk::GetDefaultDepthBufferFormat(*device)};

    auto renderPass = std::make_shared<vk::DepthColorRenderPass>(device, depthColorConfig);

    vk::ShaderCache shderCache(device);

    vk::DescriptorSetLayoutCache layoutCache(device);

    vk::PipelineLayoutCreationPreset imguiPipelineLayoutPreset(vk::PipelinePresetType::eImGui, layoutCache);
    auto imguiPipelineLayout = std::make_shared<vk::PipelineLayout>(device, imguiPipelineLayoutPreset.GetCreateInfo());

    vk::PipelineCreationPreset imguiPipelinePreset(
        vk::PipelinePresetType::eImGui, shderCache, imguiPipelineLayout->Get(), renderPass->Get());
    vk::Pipeline imguiPipeline {};

    REQUIRE_NOTHROW(imguiPipeline = vk::Pipeline(device, imguiPipelineLayout, renderPass, imguiPipelinePreset.GetCreateInfo()));
}