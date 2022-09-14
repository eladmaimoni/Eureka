#include "App.hpp"
#include <debugger_trace.hpp>
#include <RenderingSystem.hpp>
#include <AssetLoading.hpp>
#include <Window.hpp>

namespace eureka
{

    App::App()
    {

    }

    App::~App()
    {
        // TODO, still need to handle the case where the task has already been scheduled to the main thread
        // and we are currenly executing it
        // can probably solved via the session pattern
        CancelPendingOperations();
    }

    void App::CancelPendingOperations()
    {
        if (_pendingLoad && _pendingLoad.status() == concurrencpp::result_status::idle)
        {
            DEBUGGER_TRACE("cancelling pending operation");
            _cancellationSource.request_stop();
            _pendingLoad.wait();
        }
    }

    void App::Run()
    {
        Initialize();
   
        while (!_window->ShouldClose())
        {
            _window->PollEvents();
            _renderingSystem->RunOne();
        }
   
        _renderingSystem->Deinitialize();
    }

    void App::Initialize()
    {
        _renderingSystem = _container.GetRenderingSystem();
        _assetLoader = _container.CreateAssetLoader();
        _window = _container.GetWindow();

        if (!_pendingLoad)
        {

            //auto path = std::filesystem::path("C:/Projects/Samples/Vulkan/data/models/FlightHelmet/glTF/scene.gltf");

            //if (std::filesystem::exists(path))
            //{
            //    _cancellationSource = std::stop_source();
            //    _pendingLoad = _assetLoader->LoadModel(
            //        path,
            //        ModelLoadingConfig{ .cancel = _cancellationSource.get_token() }
            //    );
            //}


            
        }



    }

}