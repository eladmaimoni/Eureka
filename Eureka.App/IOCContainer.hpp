#include <GLFWRuntime.hpp>
#include <Instance.hpp>
#include <DeviceContext.hpp>

namespace eureka
{
    class RenderingSystem;

    class IOCContainer
    {
    public:
        IOCContainer();
        ~IOCContainer();


        std::unique_ptr<RenderingSystem> CreateRenderingSystem() ;

    private:
        GLFWRuntime    _glfw;
        Instance       _instance;
        DeviceContext  _deviceContext;

        

    };
}