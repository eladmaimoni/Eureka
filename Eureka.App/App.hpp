#include "IOCContainer.hpp"

namespace eureka
{


    class App
    {
    public:
        App();
        ~App();
        void Run();

    private:
        IOCContainer _container;
        std::unique_ptr<RenderingSystem> _renderingSystem;
    private:
        void Initialize();

    };
}