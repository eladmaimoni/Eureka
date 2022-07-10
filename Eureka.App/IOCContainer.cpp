#include "IOCContainer.hpp"
#include "VkHelpers.hpp"

#include "../Eureka.Graphics/RenderingSystem.hpp"
#include "../Eureka.Graphics/SubmissionThreadExecutionContext.hpp"
#include "../Eureka.Graphics/OneShotCopySubmission.hpp"
#include "../Eureka.Graphics/ImguiIntegration.hpp"
#include "../Eureka.Graphics/Window.hpp"
#include "../Eureka.Graphics/AssetLoading.hpp"
#include "../Eureka.Graphics/Descriptors.hpp"

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
        Profiling::InitProfilingCategories();

        InitializeGraphicsSubsystem();
   

    }

    IOCContainer::~IOCContainer()
    {
       
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
            _submissionThreadExecutionContext,
            _oneShotCopySubmissionHandler,
            _uploadPool,
            _pipelineCache,
            _descPool,
            _concurrencyRuntime.background_executor(),
            _concurrencyRuntime.thread_pool_executor()
            );
    }

    void IOCContainer::InitializeGraphicsSubsystem()
    {
        _deviceContext.Init(_instance, CreateDeviceContextConfig());

        _graphicsQueue = _deviceContext.CreateGraphicsQueue();
        _copyQueue = _deviceContext.CreateCopyQueue();

        auto submissionThreadExecutor = _concurrencyRuntime.make_executor<submission_thread_executor>();

        _oneShotCopySubmissionHandler = std::make_shared<OneShotCopySubmissionHandler>(_deviceContext, _copyQueue);

        _submissionThreadExecutionContext = std::make_shared<SubmissionThreadExecutionContext>(
            _deviceContext,
            _copyQueue,
            _graphicsQueue,
            std::move(submissionThreadExecutor)
            );

        _uploadPool = std::make_shared<HostWriteCombinedRingPool>(_deviceContext, STAGE_ZONE_SIZE);


        _window = std::make_shared<Window>(_glfw, _instance, _deviceContext, _graphicsQueue);

        _descPool = std::make_shared<MTDescriptorAllocator>(_deviceContext);

        auto primaryFrame = std::make_shared<SwapChainDepthColorFrame>(_deviceContext, _graphicsQueue, _window->GetSwapChain());

        _pipelineCache = std::make_shared<PipelineCache>(_deviceContext, primaryFrame->GetRenderPass());

        _renderingSystem = std::make_shared<RenderingSystem>(
            _deviceContext,
            primaryFrame,
            _pipelineCache,
            _submissionThreadExecutionContext,
            _oneShotCopySubmissionHandler,
            _descPool,
            _graphicsQueue,
            _copyQueue
            );


        _renderingSystem->Initialize();

    

        _imguiIntegration = std::make_shared<ImGuiIntegration>(
            _deviceContext,
            _pipelineCache, // TODO init from outside
            _submissionThreadExecutionContext,
            _oneShotCopySubmissionHandler,
            _uploadPool,
            _concurrencyRuntime.thread_pool_executor()
            );

       
    }

}