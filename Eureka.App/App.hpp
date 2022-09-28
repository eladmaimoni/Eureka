#include "IOCContainer.hpp"

namespace eureka
{

    class Window;
    class RemoteLiveSlamClient;
    class RemoteLiveSlamUI;


    class App
    {
    public:
        App();
        ~App();

        void Run();
        
    private:
        AppMemo      _memo;
        IOCContainer _container;
        std::shared_ptr<graphics::RenderingSystem> _renderingSystem;


    private:
        void Initialize();
        void Shutdown();
        void PollSystemEvents(std::chrono::high_resolution_clock::time_point deadline);

        std::stop_source      _cancellationSource;
        std::shared_ptr<Window> _window;
        std::shared_ptr<RemoteLiveSlamClient> _remoteHandler;
        std::shared_ptr<RemoteLiveSlamUI> _remoteUI;
    };
}    