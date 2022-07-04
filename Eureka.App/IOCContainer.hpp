#include <GLFWRuntime.hpp>
#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include <Pool.hpp>


namespace eureka
{
    class RenderingSystem;
    class AssetLoader;
    class SubmissionThreadExecutionContext;
    class OneShotCopySubmissionHandler;
    class ImGuiIntegration;
    class Window;

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
                                                          
        concurrencpp::runtime                              _concurrencyRuntime;
        std::shared_ptr<SubmissionThreadExecutionContext>  _submissionThreadExecutionContext;
        std::shared_ptr<OneShotCopySubmissionHandler>      _oneShotCopySubmissionHandler;
        std::shared_ptr<HostWriteCombinedRingPool>         _uploadPool;
        std::shared_ptr<ImGuiIntegration>                  _imguiIntegration;
        std::shared_ptr<RenderingSystem>                   _renderingSystem;
        std::shared_ptr<Window>                            _window;
        void InitializeGraphicsSubsystem();

    };
}