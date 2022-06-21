#include <GLFWRuntime.hpp>
#include <Instance.hpp>
#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include "SubmissionThreadExecutor.hpp"

namespace eureka
{
    class RenderingSystem;
    class AssetLoader;

    class IOCContainer
    {
    public:
        IOCContainer();
        ~IOCContainer();


        std::unique_ptr<RenderingSystem> CreateRenderingSystem();
        std::unique_ptr<AssetLoader> CreateAssetLoader();
    private:
        GLFWRuntime                                    _glfw;
        Instance                                       _instance;
        DeviceContext                                  _deviceContext;
        Queue                                          _graphicsQueue;
        Queue                                          _copyQueue;

        concurrencpp::runtime                           _concurrencyRuntime;
        std::shared_ptr<submission_thread_executor>     _submissionThreadExecutor;
        //OneShotCopySubmitExecutor                       _copySubmitExecutor;





    };
}