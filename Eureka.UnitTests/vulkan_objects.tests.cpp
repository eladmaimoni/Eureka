#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/RenderTarget.hpp"
#include "../Eureka.Vulkan/ResourceAllocator.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include "../Eureka.Windowing/Window.hpp"
#include <catch.hpp>
#include <debugger_trace.hpp>
#include <move.hpp>
#include <sigslot/signal.hpp>

namespace vk = eureka::vulkan;

TEST_CASE("steal", "[vulkan]")
{
    SECTION("ptr")
    {
        int val = 5;
        int* p1 = &val;

        int* p2 = eureka::steal(p1);
        CHECK(p2 == &val);
        CHECK(p1 == nullptr);
    }

    SECTION("vk handle")
    {
        auto instance = vk::MakeDefaultInstance();
        auto device = vk::MakeDefaultDevice(instance);
        auto rawDevice1 = device->GetDevice();
        auto rawDevice2 = eureka::steal(rawDevice1);
        CHECK(rawDevice1 == nullptr);
        CHECK(rawDevice2 == device->GetDevice());
    }
}

TEST_CASE("depth color render target", "[vulkan]")
{
    eureka::GLFWRuntime glfw;
    auto instance = vk::MakeDefaultInstance();
    auto window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig{});
    auto device = vk::MakeDefaultDevice(instance, window->GetSurface());
    auto allocator = std::make_shared<vk::ResourceAllocator>(instance, device);

    auto graphicsQueue = device->GetGraphicsQueue();
    auto presntationQueue = device->GetPresentQueue();

    vk::SwapChain swapChain(window, device, presntationQueue, graphicsQueue);

    vk::DepthColorRenderPassConfig depthColorConfig{.color_output_format = swapChain.ImageFormat(),
                                                    .depth_output_format = vk::GetDefaultDepthBufferFormat(*device)};

    auto renderPass = std::make_shared<vk::DepthColorRenderPass>(device, depthColorConfig);

    std::vector<vk::DepthColorRenderTarget> targets;
    REQUIRE_NOTHROW(targets = vk::CreateDepthColorTargetForSwapChain(device, allocator, swapChain, renderPass));
}
