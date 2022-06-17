#include "IOCContainer.hpp"
#include <AssetLoading.hpp>

namespace eureka
{

    class Window;


    class App
    {
    public:
        App();
        ~App();

        void CancelPendingOperations();

        void Run();

    private:
        IOCContainer _container;
        std::shared_ptr<RenderingSystem> _renderingSystem;
        std::unique_ptr<AssetLoader> _assetLoader;
    private:
        void Initialize();

        std::stop_source      _cancellationSource;
        result_t<LoadedModel> _pendingLoad;
        std::shared_ptr<Window> _window;
    };
}    