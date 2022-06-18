#include "App.hpp"
#include <debugger_trace.hpp>
#include <RenderingSystem.hpp>
#include <AssetLoading.hpp>

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
                //DEBUGGER_TRACE("app loop {}", i);
            }

   
            ++i;
        }

        _renderingSystem->Deinitialize();
    }

    void App::Initialize()
    {
        _renderingSystem = _container.CreateRenderingSystem();
        _assetLoader = _container.CreateAssetLoader();



        _renderingSystem->Initialize();


        _assetLoader->LoadModel("C:/Projects/Samples/Vulkan/data/models/FlightHelmet/glTF/scene.gltf");


    }

}