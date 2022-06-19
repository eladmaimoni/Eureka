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
            CopySubmitExecutor copySubmitExecutor,
            IOExecutor ioExecutor,
            PoolExecutor poolExecutor
            
        )
            :
            _deviceContext(deviceContext),
            _copyQueue(queue),
            _copySubmitExecutor(std::move(copySubmitExecutor)),
            _ioExecutor(std::move(ioExecutor)),
            _poolExecutor(std::move(poolExecutor))
        {

        }

        result_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config = ModelLoadingConfig{});

        std::atomic_bool              _busy{ false };
        DeviceContext&                _deviceContext;
        Queue                         _copyQueue;
        CopySubmitExecutor            _copySubmitExecutor;
        IOExecutor                    _ioExecutor;
        PoolExecutor                  _poolExecutor;
    };

}