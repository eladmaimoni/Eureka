#include <GLFWRuntime.hpp>
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"
#include <GraphicsDefaults.hpp>
#include <AppTypes.hpp>

#include "../Eureka.Graphics/RenderDocIntegration.hpp"

namespace eureka::vulkan
{
    class Instance;
    class Device;
    class SwapChain;
    class PipelineCache;
    class DescriptorSetLayoutCache;
    class ResourceAllocator;
}

namespace eureka::graphics
{
    class AsyncDataLoader;
    class RenderingSystem;
    class SubmissionThreadExecutionContext;
    class OneShotSubmissionHandler;
    class ImGuiIntegration;
    class RenderDocIntegration;
}

namespace eureka
{
    class Window;

    class AsyncDataLoader;

    namespace rpc
    {
        class RemoteLiveSlamClient;
    }
    namespace ui
    {
        class RemoteLiveSlamUI;
    }


    class IOCContainer
    {
    public:
        IOCContainer();
        ~IOCContainer();
        void Wire(AppMemo appMemo);

        std::shared_ptr<graphics::RenderingSystem> GetRenderingSystem();
        std::shared_ptr<Window> GetWindow();
        std::shared_ptr<rpc::RemoteLiveSlamClient> GetRemoteHandler();
        std::shared_ptr<ui::RemoteLiveSlamUI> GetRemoteUI();
    private:
        GLFWRuntime                                         _glfw;
        std::shared_ptr<vulkan::Instance>                   _instance;
        std::shared_ptr<vulkan::Device>                     _device;
        vulkan::Queue                                       _graphicsQueue;
        vulkan::Queue                                       _copyQueue;
        vulkan::Queue                                       _presentationQueue;
        std::shared_ptr<vulkan::ResourceAllocator>          _resourceAllocator;
        RenderDocIntegration                                _renderDocIntegration;

        concurrencpp::runtime                               _concurrencyRuntime;
        std::shared_ptr<graphics::SubmissionThreadExecutionContext>   _submissionThreadExecutionContext;
        std::shared_ptr<graphics::AsyncDataLoader>                    _asyncDataLoader;
        std::shared_ptr<graphics::OneShotSubmissionHandler>           _oneShotSubmissionHandler;

        std::shared_ptr<graphics::ImGuiIntegration>                   _imguiIntegration;
        std::shared_ptr<graphics::RenderingSystem>                    _renderingSystem;
        std::shared_ptr<Window>                             _window;
        std::shared_ptr<vulkan::SwapChain>                          _swapChain;

        //std::shared_ptr<vulkan::PipelineCache>                      _pipelineCache; // TODO per TargetPass
        std::shared_ptr<vulkan::DescriptorSetLayoutCache>           _setLayoutCache;
        std::shared_ptr<rpc::RemoteLiveSlamClient>               _remoteHandler;
        std::shared_ptr<ui::RemoteLiveSlamUI>                   _remoteUI;
        //std::shared_ptr<MTDescriptorAllocator>              _descPool;

        void InitializeGraphicsSubsystem(AppMemo& appMemo);
        void InitializeRemoteServices();

    };
}