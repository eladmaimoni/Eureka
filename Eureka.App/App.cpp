#include "App.hpp"
#include <debugger_trace.hpp>
#include <RenderingSystem.hpp>

namespace eureka
{

    App::App()
    {

    }

    App::~App()
    {

    }

    void App::Run()
    {
        Initialize();
   
        auto i = 0;
        while (!glfwWindowShouldClose(_renderingSystem->WindowHandle()))
        {
            glfwPollEvents();
            _renderingSystem->RunOne();
            //std::this_thread::sleep_for(100ms);
            if (0 == (i % 100))
            {
                DEBUGGER_TRACE("app loop {}", i);
            }

   
            ++i;
        }
    }

    void App::Initialize()
    {
        _renderingSystem = _container.CreateRenderingSystem();


        _renderingSystem->Initialize();
    }

}