
#include <catch.hpp>
#include <debugger_trace.hpp>
#include <macros.hpp>
#include <sigslot/signal.hpp>
#include "../Eureka.Windowing/Window.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"

namespace vk = eureka::vulkan;


TEST_CASE("vulkan instance creation", "[vulkan]")
{

    std::shared_ptr<vk::Instance> instance;

    REQUIRE_NOTHROW(instance = vk::MakeDefaultInstance());
}


TEST_CASE("vulkan device creation", "[vulkan]")
{
    std::shared_ptr<vk::Device> device;

    vk::InstanceConfig config{};
    config.required_layers.emplace_back(vk::INSTANCE_LAYER_VALIDATION);
    config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_DEBUG_UTILS);
    config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
    WIN32_ONLY(config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME));

    SECTION("vulkan < 1.3")
    {
        config.version = vk::Version{ 1,2,0 };
        config.required_layers.emplace_back(vk::INSTANCE_LAYER_PRE13_SYNCHRONIZATION2);
        auto instance = std::make_shared<vk::Instance>(config);
        REQUIRE_NOTHROW(device = vk::MakeDefaultDevice(instance));
    }

    SECTION("installed version")
    {
        auto instance = std::make_shared<vk::Instance>(config);
        REQUIRE_NOTHROW(device = vk::MakeDefaultDevice(instance));
    }
}


TEST_CASE("vulkan queue creation", "[vulkan]")
{ 
    vk::InstanceConfig config{};
    config.required_layers.emplace_back(vk::INSTANCE_LAYER_VALIDATION);
    config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_DEBUG_UTILS);
    config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
    WIN32_ONLY(config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME));

    auto instance = std::make_shared<vk::Instance>(config);
    auto device = vk::MakeDefaultDevice(instance);
  
    auto graphicsQueue = device->GetGraphicsQueue();
    auto copyQueue = device->GetCopyQueue();
    auto computeQueue = device->GetComputeQueue();

    auto prettyName = device->GetPrettyName();

    if (prettyName == "NVIDIA GeForce RTX 3060 Ti")
    {
        CHECK(graphicsQueue.Family() == 0);
        CHECK(copyQueue.Family() == 1);
        CHECK(computeQueue.Family() == 2);
    }
    else if (prettyName == "Intel(R) UHD Graphics 750")
    {
        CHECK(graphicsQueue.Family() == 0);
        CHECK(copyQueue.Family() == 0);
        CHECK(computeQueue.Family() == 0);
    }
    else
    {
        CHECK(true);
    }

}

TEST_CASE("vulkan queue creation with presentation", "[vulkan]")
{
    eureka::GLFWRuntime glfw;

    vk::InstanceConfig config{};
    config.required_layers.emplace_back(vk::INSTANCE_LAYER_VALIDATION);
    DEBUG_ONLY(config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_DEBUG_UTILS));
    config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
    WIN32_ONLY(config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME));

    auto instance = std::make_shared<vk::Instance>(config);
    std::shared_ptr<vk::Device> device;
    auto window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig{});
    device = vk::MakeDefaultDevice(instance, window->GetSurface());


    auto graphicsQueue = device->GetGraphicsQueue();
    auto copyQueue = device->GetCopyQueue();
    auto computeQueue = device->GetComputeQueue();
    auto presntationQueue = device->GetPresentQueue();


    vk::SwapChain sp(window, device, presntationQueue, graphicsQueue);

    //window.reset(); // instance must outlive the window

    //while (!window->ShouldClose())
    //{
    //    window->PollEvents();
    //}
    auto prettyName = device->GetPrettyName();

    if (prettyName == "NVIDIA GeForce RTX 3060 Ti")
    {
        CHECK(graphicsQueue.Family() == 0);
        CHECK(presntationQueue.Family() == 2);
        CHECK(copyQueue.Family() == 1);
        CHECK(computeQueue.Family() == 2);
    }
    else if (prettyName == "Intel(R) UHD Graphics 750")
    {
        CHECK(graphicsQueue.Family() == 0);
        CHECK(presntationQueue.Family() == 0);
        CHECK(copyQueue.Family() == 0);
        CHECK(computeQueue.Family() == 0);
    }
    else
    {
        CHECK(true);
    }

}


