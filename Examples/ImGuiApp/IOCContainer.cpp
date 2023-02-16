#include "IOCContainer.hpp"

#include "../Eureka.Graphics/ImguiIntegration.hpp"
#include "../Eureka.Graphics/OneShotCopySubmission.hpp"
#include "../Eureka.Graphics/RenderingSystem.hpp"
#include "../Eureka.Graphics/SubmissionThreadExecutionContext.hpp"
#include "../Eureka.Vulkan/BufferMemoryPool.hpp"

#include "../Eureka.Windowing/Window.hpp"

#include "../Eureka.Graphics/ImGuiViewPass.hpp"
#include "../Eureka.Graphics/TargetPass.hpp"
//#include "../Eureka.Graphics/CameraPass.hpp"

EUREKA_MSVC_WARNING_PUSH_DISABLE(4005) // warning C4005 : 'APIENTRY' : macro redefinition
// TODO BUG BUG
#define MemoryBarrier __faststorefence
#include "../Eureka.RemoteClient/RemoteLiveSlamUI.hpp"
EUREKA_MSVC_WARNING_POP
#include <profiling.hpp>

namespace eureka
{
#ifdef NDEBUG
    constexpr bool IS_DEBUG_BUILD = false;
#else
    constexpr bool IS_DEBUG_BUILD = true;
#endif

    vulkan::InstanceConfig CreateInstanceConfig(const GLFWRuntime& glfw)
    {
        vulkan::InstanceConfig runtime_desc {};
        runtime_desc.version = vulkan::Version{ 1, 3, 0 };
        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_instance_extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        //runtime_desc.required_instance_extentions.emplace_back("VK_LAYER_KHRONOS_synchronization2");
        //runtime_desc.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");

        if constexpr(IS_DEBUG_BUILD)
        {
            runtime_desc.required_layers.emplace_back(eureka::vulkan::INSTANCE_LAYER_VALIDATION);
        }

        return runtime_desc;
    }

    // auto swapchainSupport = QuerySwapchainSupport(&*(_deviceContext.PhysicalDevice()), _window->GetSurface());
    //DeviceContextConfig CreateDeviceContextConfig(vk::SurfaceKHR presentationSurface = nullptr)
    //{
    //    DeviceContextConfig device_context_desc{};
    //    device_context_desc.presentation_surface = presentationSurface;
    //    //device_context_desc.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
    //    device_context_desc.required_extentions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    //    //device_context_desc.required_extentions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    //    DEBUG_ONLY(device_context_desc.required_layers.emplace_back(VK_LAYER_VALIDATION));
    //    return device_context_desc;

    //}

    IOCContainer::IOCContainer() :
        _instance(vulkan::MakeDefaultInstance(vulkan::Version{ 1, 3, 0 }))
    {}

    IOCContainer::~IOCContainer()
    {
        RenderDockIntegrationInstance = nullptr;
    }

    void IOCContainer::Wire(AppMemo appMemo)
    {
        RenderDockIntegrationInstance = &_renderDocIntegration;

        InitializeRemoteServices();
        InitializeGraphicsSubsystem(appMemo);
    }

    std::shared_ptr<graphics::RenderingSystem> IOCContainer::GetRenderingSystem()
    {
        return _renderingSystem;
    }

    std::shared_ptr<eureka::Window> IOCContainer::GetWindow()
    {
        return _window;
    }

    std::shared_ptr<rpc::RemoteLiveSlamClient> IOCContainer::GetRemoteHandler()
    {
        return _remoteHandler;
    }

    std::shared_ptr<ui::RemoteLiveSlamUI> IOCContainer::GetRemoteUI()
    {
        return _remoteUI;
    }

    void IOCContainer::InitializeGraphicsSubsystem(AppMemo& appMemo)
    {
        _window = std::make_shared<Window>(_glfw, _instance->Get(), appMemo.window_config);

        _device = vulkan::MakeDefaultDevice(_instance, _window->GetSurface());

        _graphicsQueue = _device->GetGraphicsQueue();
        _copyQueue = _device->GetCopyQueue();
        _presentationQueue = _device->GetPresentQueue();
        _resourceAllocator = std::make_shared<vulkan::ResourceAllocator>(_instance, _device);

        auto winWidth = _window->GetWidth();
        auto winHeight = _window->GetHeight();
        auto swapchainSupport = _device->GetSwapChainSupport(_window->GetSurface());
        auto capabilities = swapchainSupport.capabilities;

        if(capabilities.maxImageExtent.width < winWidth || capabilities.maxImageExtent.height < winHeight)
        {
            // HACK HACK HACK
            _window->Resize(capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
            _swapChain = std::make_shared<vulkan::SwapChain>(_window, _device, _presentationQueue, _graphicsQueue);
            _window->Resize(winWidth, winHeight);
        }
        else
        {
            _swapChain = std::make_shared<vulkan::SwapChain>(_window, _device, _presentationQueue, _graphicsQueue);
        }
        // HACK HACK HACK

        auto submissionThreadExecutor = _concurrencyRuntime.make_executor<submission_thread_executor>();

        _setLayoutCache = std::make_shared<vulkan::DescriptorSetLayoutCache>(_device);
        _submissionThreadExecutionContext = std::make_shared<graphics::SubmissionThreadExecutionContext>(
            _device, _copyQueue, _graphicsQueue, std::move(submissionThreadExecutor));

        //_descPool = std::make_shared<MTDescriptorAllocator>(_deviceContext);

        _imguiIntegration = std::make_shared<graphics::ImGuiIntegration>();
        _imguiIntegration->BindToGLFWWindow(_window->GetWindowHandle());
        auto frameContext = std::make_shared<vulkan::FrameContext>(_device, _copyQueue, _graphicsQueue);

        _oneShotSubmissionHandler = std::make_shared<graphics::OneShotSubmissionHandler>(
            _device, _copyQueue, _graphicsQueue, frameContext, _submissionThreadExecutionContext);

        auto uploadPool = std::make_shared<vulkan::BufferMemoryPool>(
            _resourceAllocator,
            1024 * 1024 * 8,
            VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
            VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT

        );

        _asyncDataLoader = std::make_shared<graphics::AsyncDataLoader>(_oneShotSubmissionHandler, uploadPool);

        auto shaderCache = std::make_shared<vulkan::ShaderCache>(_device);
        auto layoutCache = std::make_shared<vulkan::DescriptorSetLayoutCache>(_device);
        auto descriptorAllocator = std::make_shared<vulkan::FreeableDescriptorSetAllocator>(_device);

        graphics::GlobalInheritedData globalInheritedData {
            .device = _device,
            .resource_allocator = _resourceAllocator,
            .shader_cache = shaderCache,
            .layout_cache = layoutCache,
            .descriptor_allocator = descriptorAllocator,
            .async_data_loader = _asyncDataLoader,
        };

        auto colorPass =
            std::make_shared<graphics::SwapChainDepthColorPass>(globalInheritedData, _graphicsQueue, _swapChain);

        //_pipelineCache = std::make_shared<PipelineCache>(_device, _setLayoutCache, colorPass->GetRenderPass());

        _remoteUI = std::make_shared<ui::RemoteLiveSlamUI>(std::move(appMemo.liveslam), _remoteHandler);

        auto imguiPass = std::make_shared<graphics::ImGuiViewPass>(globalInheritedData, _remoteUI);

        colorPass->AddViewPass(imguiPass);

        _renderingSystem = std::make_shared<graphics::RenderingSystem>(_device,
                                                                       _graphicsQueue,
                                                                       _copyQueue,
                                                                       frameContext,
                                                                       colorPass,
                                                                       _submissionThreadExecutionContext,
                                                                       _oneShotSubmissionHandler);

        _renderingSystem->Initialize();
    }

    void IOCContainer::InitializeRemoteServices()
    {
        _remoteHandler = std::make_shared<rpc::RemoteLiveSlamClient>();
    }

} // namespace eureka