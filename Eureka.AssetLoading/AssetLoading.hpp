#pragma once

#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include <SecondaryCommandRecorder.hpp>
#include <Commands.hpp>
#include <SubmissionThreadExecutionContext.hpp>

namespace eureka
{



    struct ModelLoadingConfig
    {
        std::stop_token cancel;
    };
    struct LoadedModel
    {

    };

    inline constexpr uint64_t STAGE_ZONE_SIZE = 1024 * 1024 * 400; // 20MB

    class AssetLoader
    {
    public:
        AssetLoader(
            DeviceContext& deviceContext,
            Queue queue,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            IOExecutor ioExecutor,
            PoolExecutor poolExecutor     
        );

        result_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config);

        std::atomic_bool                                     _busy{ false };
        DeviceContext&                                       _deviceContext;
        Queue                                                _copyQueue;
        std::shared_ptr<SubmissionThreadExecutionContext>    _submissionThreadExecutionContext;
        IOExecutor                                           _ioExecutor;
        PoolExecutor                                         _poolExecutor;
        SequentialStageZone                                  _stageZone;
        CommandPool                                          _uploadCommandPool;
    };

}