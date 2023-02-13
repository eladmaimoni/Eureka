#include <debugger_trace.hpp>
#include <sigslot/signal.hpp>
#ifdef PERFETTO_TRACING
#include "perfetto_tracing_session.hpp"
#endif
#include <profiling.hpp>
#include <system.hpp> 
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
namespace prof = eureka::profiling;


static_assert(FLUTTER_ENGINE_VERSION == 1,
    "This Flutter Embedder was authored against the stable Flutter "
    "API at version 1. There has been a serious breakage in the "
    "API. Please read the ChangeLog and take appropriate action "
    "before updating this assertion"
    );



#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif
/*
invalid options:
--target-platform=windows

cd /c/workspace/ultrawis
flutter build bundle --debug --local-engine-src-path=/c/Libraries/engine/src --local-engine=/c/Libraries/engine/src/out/host_debug_unopt
*/

const std::filesystem::path FLUTTER_EXAMPLE_PROJECT_OUT_PATH = "C:/workspace/flutter_ultwrawis_iot";
const std::filesystem::path FLUTTER_EXAMPLE_PROJECT_ASSETS_PATH = FLUTTER_EXAMPLE_PROJECT_OUT_PATH / "flutter_assets";
const std::filesystem::path FLUTTER_EXAMPLE_PROJECT_ICUDTL_PATH = FLUTTER_EXAMPLE_PROJECT_OUT_PATH / "icudtl.dat";
const std::filesystem::path FLUTTER_EXAMPLE_PROJECT_AOT_PATH = FLUTTER_EXAMPLE_PROJECT_OUT_PATH / "app.so";

int main(int argc, char* argv[])
{
    eureka::os::set_system_timer_frequency(1ms);
#ifdef PERFETTO_TRACING
    std::unique_ptr<eureka::PerfettoTracing> perfettoTracing;
    if (argc >= 2 && argv)
    {
        if (argv[1] == std::string("-prof"))
        {
            eureka::PerfettoTracingConfig profiling_config{};

            perfettoTracing = std::make_unique<eureka::PerfettoTracing>(std::move(profiling_config));
            perfettoTracing->StartTracing();
        }
    }
#endif
    try
    {
        PROFILE_PUSH_CATEGORIZED_RANGE("System Initialization", prof::Color::Blue, prof::PROFILING_CATEGORY_SYSTEM);


        eureka::GLFWRuntime glfw;


        auto instance = vk::MakeDefaultInstance(vk::Version{ 1,2,0 });

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


        fl::EmbedderConfig embedderConfig
        {
            .asset_dir = FLUTTER_EXAMPLE_PROJECT_ASSETS_PATH,
            .icudtl_path = FLUTTER_EXAMPLE_PROJECT_ICUDTL_PATH,
            .aot_path = FLUTTER_EXAMPLE_PROJECT_AOT_PATH,
        };

        auto flutterCompositor = std::make_shared<fl::FlutterVulkanCompositor>(instance, globalInheritedData, frameContext, depthColorTarget);

        auto flutterProjectEmbedder = std::make_shared<fl::Embedder>(embedderConfig, flutterCompositor, window);

        flutterProjectEmbedder->Run();
        
        PROFILE_POP_RANGE(prof::PROFILING_CATEGORY_SYSTEM);
        flutterProjectEmbedder->Loop();


    }
    catch (const std::exception& err)
    {
        DEBUGGER_TRACE("{}", err.what());
    }
    return 0;
}

