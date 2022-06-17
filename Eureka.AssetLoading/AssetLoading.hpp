#pragma once

#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>


namespace eureka
{



    struct ModelLoadingConfig
    {
       
    };
    struct LoadedModel
    {

    };



    class AssetLoader
    {
    public:
        AssetLoader(
            DeviceContext& deviceContext,
            Queue queue,
            CopySubmitExecutor copySubmitExecutor
        )
            :
            _deviceContext(deviceContext),
            _copyQueue(queue),
            _copySubmitExecutor(std::move(copySubmitExecutor))
        {

        }

        LoadedModel LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config = ModelLoadingConfig{});


        DeviceContext& _deviceContext;
        Queue                         _copyQueue;
        CopySubmitExecutor            _copySubmitExecutor;
    };

}