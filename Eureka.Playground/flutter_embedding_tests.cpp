#include <catch.hpp>
#include <debugger_trace.hpp>
#include <sigslot/signal.hpp>
#include "../Eureka.Windowing/Window.hpp"
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include "../Eureka.Vulkan/FrameContext.hpp"
//#include "../Eureka.Vulkan/RenderTarget.hpp"
#include "../Eureka.Graphics/TargetPass.hpp"
#include "../Eureka.Flutter/FlutterVulkanCompositor.hpp"
#include "../Eureka.Flutter/FlutterProjectEmbedder.hpp"



namespace vk = eureka::vulkan;
namespace fl = eureka::flutter;

static_assert(FLUTTER_ENGINE_VERSION == 1,
    "This Flutter Embedder was authored against the stable Flutter "
    "API at version 1. There has been a serious breakage in the "
    "API. Please read the ChangeLog and take appropriate action "
    "before updating this assertion"
    );



/*
invalid options:
--target-platform=windows

cd /c/workspace/ultrawis
flutter build bundle --debug --local-engine-src-path=/c/Libraries/engine/src --local-engine=/c/Libraries/engine/src/out/host_debug_unopt 
*/


TEST_CASE("flutter engine debug mode", "[flutter][vulkan]")
{
    
    try
    {
        eureka::GLFWRuntime glfw;

        vk::InstanceConfig config{};
        config.version = vk::Version{ 1,2,170 };
        config.required_layers.emplace_back(vk::INSTANCE_LAYER_VALIDATION);
        //config.required_layers.emplace_back(vk::INSTANCE_LAYER_PRE13_SYNCHRONIZATION2);
        config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_DEBUG_UTILS);
        config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
        WIN32_ONLY(config.required_instance_extentions.emplace_back(vk::INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME));


        auto instance = std::make_shared<vk::Instance>(config);

        auto window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig{});
        auto device = vk::MakeDefaultDevice(instance, window->GetSurface());
        auto resourceAllocator = std::make_shared<vk::ResourceAllocator>(instance, device);
        auto graphicsQueue = device->GetGraphicsQueue();
        auto presntationQueue = device->GetPresentQueue();
        auto copyQueue = device->GetCopyQueue();

        auto swapChain = std::make_shared<vk::SwapChain>(window, device, presntationQueue, graphicsQueue);


        //auto asyncDataLoader = std::make_shared<graphics::AsyncDataLoader>(_oneShotSubmissionHandler, uploadPool);

        auto shaderCache = std::make_shared<eureka::vulkan::ShaderCache>(device);
        auto layoutCache = std::make_shared<eureka::vulkan::DescriptorSetLayoutCache>(device);
        auto descriptorAllocator = std::make_shared<eureka::vulkan::FreeableDescriptorSetAllocator>(device);

        eureka::graphics::GlobalInheritedData globalInheritedData{
            .device = device,
            .resource_allocator = resourceAllocator,
            .shader_cache = shaderCache,
            .layout_cache = layoutCache,
            .descriptor_allocator = descriptorAllocator,
            .async_data_loader = nullptr,
        };



        auto depthColorTarget = std::make_shared<eureka::graphics::SwapChainDepthColorPass>(globalInheritedData, graphicsQueue, swapChain);
        auto frameContext = std::make_shared<vk::FrameContext>(device, copyQueue, graphicsQueue);
    



        auto flutterCompositor = std::make_shared<fl::FlutterVulkanCompositor>(instance, globalInheritedData, frameContext, depthColorTarget);

        auto flutterProjectEmbedder = std::make_shared<fl::FlutterProjectEmbedder>(flutterCompositor, window);

        flutterProjectEmbedder->Run();

        while (!window->ShouldClose())
        {
            window->PollEvents();
        
            flutterProjectEmbedder->DoTasks();
        }

    }
    catch (const std::exception& err)
    {
        DEBUGGER_TRACE("{}", err.what());
    }
    
}