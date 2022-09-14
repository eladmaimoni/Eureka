#include <GLFWRuntime.hpp>
#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include <Pool.hpp>
#include "../Eureka.Graphics/RenderDocIntegration.hpp"

namespace eureka
{
    class RenderingSystem;
    class AssetLoader;
    class SubmissionThreadExecutionContext;
    class OneShotSubmissionHandler;
    class ImGuiIntegration;
    class Window;
    class MTDescriptorAllocator;
    class PipelineCache;
    class RenderDocIntegration;
    class DescriptorSetLayoutCache;
    class AsyncDataLoader;

    class IOCContainer
    {
    public:
        IOCContainer();
        ~IOCContainer();


        std::shared_ptr<RenderingSystem> GetRenderingSystem();
        std::shared_ptr<Window> GetWindow();
        std::unique_ptr<AssetLoader> CreateAssetLoader();
    private:
        GLFWRuntime                                        _glfw;
        Instance                                           _instance;
        DeviceContext                                      _deviceContext;
        Queue                                              _graphicsQueue;
        Queue                                              _copyQueue;
        RenderDocIntegration                               _renderDocIntegration;

        concurrencpp::runtime                              _concurrencyRuntime;
        std::shared_ptr<SubmissionThreadExecutionContext>  _submissionThreadExecutionContext;
        std::shared_ptr<AsyncDataLoader>                   _asyncDataLoader;
        std::shared_ptr<OneShotSubmissionHandler>          _oneShotSubmissionHandler;
        std::shared_ptr<HostWriteCombinedRingPool>         _uploadPool;
        std::shared_ptr<ImGuiIntegration>                  _imguiIntegration;
        std::shared_ptr<RenderingSystem>                   _renderingSystem;
        std::shared_ptr<Window>                            _window;
        std::shared_ptr<PipelineCache>                     _pipelineCache; // TODO per TargetPass
        std::shared_ptr<DescriptorSetLayoutCache>          _setLayoutCache;

        std::shared_ptr<MTDescriptorAllocator>             _descPool;
        void InitializeGraphicsSubsystem();

    };
}