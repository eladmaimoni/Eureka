#include <catch.hpp>
#include <debugger_trace.hpp>
#include <sigslot/signal.hpp>
#include "../Eureka.Windowing/Window.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include "flutter/flutter_embedder.h"

namespace vk = eureka::vulkan;

static_assert(FLUTTER_ENGINE_VERSION == 1,
    "This Flutter Embedder was authored against the stable Flutter "
    "API at version 1. There has been a serious breakage in the "
    "API. Please read the ChangeLog and take appropriate action "
    "before updating this assertion"
    );

void* FlutterGetInstanceProcAddressCallback(
    //void* user_data,
    FlutterVulkanInstanceHandle instance,
    const char* procname
) 
{
    auto* proc = glfwGetInstanceProcAddress(reinterpret_cast<VkInstance>(instance), procname);
    return reinterpret_cast<void*>(proc);
}


//FlutterVulkanImage FlutterGetNextImageCallback(
//    void* user_data,
//    const FlutterFrameInfo* frame_info
//) 
//{
//    //auto swapChain = static_cast<vk::SwapChain*>(user_data);
//
//    //auto valid = false;
//    //auto nextImage 
//    //
//    //while (!valid)
//    //{
//    //    auto [spValid, currentFrame, imageReadySemaphore] = swapChain->AcquireNextAvailableImageAsync();
//
//
//    //}
//
//
//    // If the GLFW framebuffer has been resized, discard the swapchain and create
//    // a new one.
//    if (!valid)
//    {
//        InitializeSwapchain();
//    }
//
//    d.vkAcquireNextImageKHR(g_state.device, g_state.swapchain, UINT64_MAX,
//        nullptr, g_state.image_ready_fence,
//        &g_state.last_image_index);
//
//    // Flutter Engine expects the image to be available for transitioning and
//    // attaching immediately, and so we need to force a host sync here before
//    // returning.
//    d.vkWaitForFences(g_state.device, 1, &g_state.image_ready_fence, true,
//        UINT64_MAX);
//    d.vkResetFences(g_state.device, 1, &g_state.image_ready_fence);
//
//    return {
//        .struct_size = sizeof(FlutterVulkanImage),
//        .image = reinterpret_cast<uint64_t>(
//            g_state.swapchain_images[g_state.last_image_index]),
//        .format = g_state.surface_format.format,
//    };
//}

TEST_CASE("flutter engine init", "[flutter][vulkan]")
{
    eureka::GLFWRuntime glfw;

    //instanceConfig.required_layers.emplace_back(vk::INSTANCE_LAYER_VALIDATION);
    //instanceConfig.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_DEBUG_UTILS);
    //instanceConfig.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
    //instanceConfig.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME);
    //instanceConfig.version = vk::Version{ 1,2,0 };
    //instanceConfig.required_layers.emplace_back(vk::INSTANCE_LAYER_PRE13_SYNCHRONIZATION2);

    auto instance = vk::MakeDefaultInstance();
    auto window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig{});
    auto device = vk::MakeDefaultDevice(instance, window->GetSurface());

    auto graphicsQueue = device->GetGraphicsQueue();
    auto presntationQueue = device->GetPresentQueue();

    vk::SwapChain swapChain(window, device, presntationQueue, graphicsQueue);

    //FlutterEngine engine;
    
    FlutterRendererConfig flutterRendererConfig{};
    flutterRendererConfig.type = FlutterRendererType::kVulkan;

    flutterRendererConfig.vulkan.struct_size = sizeof(flutterRendererConfig.vulkan);
    flutterRendererConfig.vulkan.version = instance->ApiVersion().Get();
    flutterRendererConfig.vulkan.instance = instance->Get();
    flutterRendererConfig.vulkan.physical_device = device->GetPhysicalDevice();
    flutterRendererConfig.vulkan.device = device->GetDevice();
    flutterRendererConfig.vulkan.queue_family_index = graphicsQueue.Family();
    flutterRendererConfig.vulkan.queue = graphicsQueue.Get();
    flutterRendererConfig.vulkan.enabled_instance_extension_count = instance->EnabledExtentions().size();
    flutterRendererConfig.vulkan.enabled_instance_extensions = instance->EnabledExtentions().data();
    flutterRendererConfig.vulkan.enabled_device_extension_count = device->EnabledExtentions().size();
    flutterRendererConfig.vulkan.enabled_device_extensions = device->EnabledExtentions().data();
    //flutterRendererConfig.vulkan.get_instance_proc_address_callback = FlutterGetInstanceProcAddressCallback;
    //flutterRendererConfig.vulkan.get_next_image_callback = FlutterGetNextImageCallback;
    //config.vulkan.present_image_callback = FlutterPresentCallback;

}