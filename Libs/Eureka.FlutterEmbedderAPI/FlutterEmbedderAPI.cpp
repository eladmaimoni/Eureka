
#include "FlutterEmbedderAPI.h"
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

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

class Main
{
	eureka::GLFWRuntime glfw;
    std::shared_ptr<fl::Embedder> _embedder;
public:

	Main(const EmbedderInitParams* params)
	{
		eureka::os::set_system_timer_frequency(1ms);

        auto instance = vk::MakeDefaultInstance(vk::Version{ 1,2,0 });

        auto window = std::make_shared<eureka::Window>(glfw, instance->Get(), eureka::WindowConfig{});
        auto device = vk::MakeDefaultDevice(instance, window->GetSurface());
        auto resourceAllocator = std::make_shared<vk::ResourceAllocator>(instance, device);
        auto graphicsQueue = device->GetGraphicsQueue();
        auto presntationQueue = device->GetPresentQueue();
        auto copyQueue = device->GetCopyQueue();

        auto swapChain = std::make_shared<vk::SwapChain>(window, device, presntationQueue, graphicsQueue);


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
            .asset_dir = params->asset_path,
            .icudtl_path = params->icudtl_path,
        };

        auto flutterCompositor = std::make_shared<fl::FlutterVulkanCompositor>(instance, globalInheritedData, frameContext, depthColorTarget);

        _embedder = std::make_shared<fl::Embedder>(embedderConfig, flutterCompositor, window);

	}

    void Run()
    {
        _embedder->Run();

        _embedder->Loop();
    }
};


FlutterEmbedderStatus FlutterEmbedderInit(const EmbedderInitParams* params, EMBEDDER_HANDLE* out)
{
    try
    {
        auto instance = new Main(params);
        *out = instance;
        return FlutterEmbedderStatus::eOk;
    }
    catch (...)
    {
        return FlutterEmbedderStatus::eFail;
    }
}
FlutterEmbedderStatus FlutterEmbedderFinalize(EMBEDDER_HANDLE embedder)
{
    try
    {
        delete static_cast<Main*>(embedder);
        return FlutterEmbedderStatus::eOk;
    }
    catch (...)
    {
        return FlutterEmbedderStatus::eFail;
    }

}
FlutterEmbedderStatus FlutterEmbedderRun(EMBEDDER_HANDLE embedder)
{
    try
    {
        static_cast<Main*>(embedder)->Run();
        return FlutterEmbedderStatus::eOk;
    }
    catch (...)
    {
        return FlutterEmbedderStatus::eFail;
    }

}