#pragma once

#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include <SecondaryCommandRecorder.hpp>
#include <Commands.hpp>

namespace eureka
{



    struct ModelLoadingConfig
    {
       
    };
    struct LoadedModel
    {

    };

    inline constexpr uint64_t STAGE_ZONE_SIZE = 1024 * 1024 * 4; // 4MB

    class AssetLoader
    {
    public:
        AssetLoader(
            DeviceContext& deviceContext,
            Queue queue,
            CopySubmitExecutor copySubmitExecutor,
            IOExecutor ioExecutor,
            PoolExecutor poolExecutor
            
        );

        result_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config = ModelLoadingConfig{});

        std::atomic_bool                     _busy{ false };
        DeviceContext&                       _deviceContext;
        Queue                                _copyQueue;
        CopySubmitExecutor                   _copySubmitExecutor;
        IOExecutor                           _ioExecutor;
        PoolExecutor                         _poolExecutor;
        SequentialStageZone                  _stageZone;
        CommandPool                          _uploadCommandPool;
    };

}