#include "IOCContainer.hpp"
#include "VkHelpers.hpp"

#include "../Eureka.Graphics/RenderingSystem.hpp"
#include "../Eureka.Graphics/SubmissionThreadExecutionContext.hpp"
#include "../Eureka.Graphics/OneShotCopySubmission.hpp"
#include "../Eureka.Graphics/ImguiIntegration.hpp"
#include "../Eureka.Graphics/Window.hpp"
#include "../Eureka.Graphics/AssetLoading.hpp"
#include "../Eureka.Graphics/Descriptors.hpp"
#include "../Eureka.Graphics/ImGuiRenderer.hpp"


namespace eureka
{
    InstanceConfig CreateInstanceConfig(const GLFWRuntime& glfw)
    {
        InstanceConfig runtime_desc{};


        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_instance_extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        runtime_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);
        runtime_desc.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");

        DEBUGGER_TRACE("Requested instance extentions = {}", runtime_desc.required_instance_extentions);
        DEBUGGER_TRACE("Requested instance layers = {}", runtime_desc.required_layers);

        return runtime_desc;
    }

    DeviceContextConfig CreateDeviceContextConfig(vk::SurfaceKHR presentationSurface = nullptr)
    {
        DeviceContextConfig device_context_desc{};
        device_context_desc.presentation_surface = presentationSurface;
        device_context_desc.required_layers.emplace_back(VK_LAYER_VALIDATION);
        device_context_desc.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
        

        device_context_desc.required_extentions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        device_context_desc.required_extentions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        DEBUGGER_TRACE("Requested device extentions = {}", device_context_desc.required_extentions);
        DEBUGGER_TRACE("Requested device layers = {}", device_context_desc.required_layers);
        return device_context_desc;


    }

    IOCContainer::IOCContainer()
        : 
        _instance(CreateInstanceConfig(_glfw))

    {
        RenderDockIntegrationInstance = &_renderDocIntegration;
        Profiling::InitProfilingCategories();

        InitializeGraphicsSubsystem();
   

    }

    IOCContainer::~IOCContainer()
    {
        RenderDockIntegrationInstance = nullptr;
    }

    std::shared_ptr<RenderingSystem> IOCContainer::GetRenderingSystem() 
    {
        return _renderingSystem;
    }

    std::shared_ptr<eureka::Window> IOCContainer::GetWindow()
    {
        return _window;
    }

    std::unique_ptr<AssetLoader> IOCContainer::CreateAssetLoader()
    {
        return std::make_unique<AssetLoader>(
            _deviceContext,
            _copyQueue,
            _oneShotSubmissionHandler,
            _uploadPool,
            _pipelineCache,
            _descPool,
            _concurrencyRuntime.background_executor(),
            _concurrencyRuntime.thread_pool_executor()
            );
    }

    future_t<void> IOCContainer::InitializeGraphicsSubsystem()
    {
        _deviceContext.Init(_instance, CreateDeviceContextConfig());

        _graphicsQueue = _deviceContext.CreateGraphicsQueue();
        _copyQueue = _deviceContext.CreateCopyQueue();
        //_copyQueue = _graphicsQueue; // HACK
        auto submissionThreadExecutor = _concurrencyRuntime.make_executor<submission_thread_executor>();


        _setLayoutCache = std::make_shared<DescriptorSetLayoutCache>(_deviceContext.LogicalDevice());
        _submissionThreadExecutionContext = std::make_shared<SubmissionThreadExecutionContext>(
            _deviceContext,
            _copyQueue,
            _graphicsQueue,
            std::move(submissionThreadExecutor)
            );


        _uploadPool = std::make_shared<HostWriteCombinedRingPool>(_deviceContext, STAGE_ZONE_SIZE);


        _window = std::make_shared<Window>(_glfw, _instance, _deviceContext, _graphicsQueue);

        _descPool = std::make_shared<MTDescriptorAllocator>(_deviceContext);

        auto primaryFrame = std::make_shared<SwapChainFrameContext>(_deviceContext, _copyQueue, _graphicsQueue, _window->GetSwapChain());

        _oneShotSubmissionHandler = std::make_shared<OneShotSubmissionHandler>(_deviceContext, _copyQueue, _graphicsQueue, primaryFrame, _submissionThreadExecutionContext);

        _pipelineCache = std::make_shared<PipelineCache>(_deviceContext, _setLayoutCache, primaryFrame->GetRenderPass());

        _imguiIntegration = std::make_shared<ImGuiIntegration>();

        auto imguiRenderer = std::make_shared<ImGuiRenderer>(
            _deviceContext,
            _window,
            _pipelineCache,
            _descPool,
            _oneShotSubmissionHandler,
            _uploadPool,
            _concurrencyRuntime.thread_pool_executor()
            );

        _renderingSystem = std::make_shared<RenderingSystem>(
            _deviceContext,
            primaryFrame,
            _pipelineCache,
            imguiRenderer,
            _submissionThreadExecutionContext,
            _oneShotSubmissionHandler,
            _descPool,
            _graphicsQueue,
            _copyQueue
            );



        _renderingSystem->Initialize();




        co_return;

    }

}