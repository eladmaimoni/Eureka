#pragma once

#include "DeviceContext.hpp"
#include "GraphicsDefaults.hpp"
#include "Commands.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "UploadRingBuffer.hpp"
#include "CommandsUtils.hpp"
#include "Mesh.hpp"
#include "Pipeline.hpp"

namespace eureka
{
    template<typename T>
    using dynamic_span = std::span<T, std::dynamic_extent>;

    template<typename T>
    using dynamic_cspan = std::span<const T, std::dynamic_extent>;

    template<typename T>
    dynamic_cspan<uint8_t> to_raw_span(const dynamic_cspan<T>& s)
    {
        return dynamic_cspan<uint8_t>(
            reinterpret_cast<const uint8_t*>(s.data()),
            s.size_bytes()
            );
    }



    struct BufferDataUploadTransferDesc
    {
        vk::Buffer src_buffer;
        uint64_t   src_offset;
        uint64_t   bytes;

        vk::Buffer dst_buffer;
        uint64_t   dst_offset;
    };

    struct ModelLoadingConfig
    {
        std::stop_token cancel;
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
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
            std::shared_ptr<HostWriteCombinedRingPool>        uploadPool,
            std::shared_ptr<PipelineCache >                   pipelineCache,
            std::shared_ptr<MTDescriptorAllocator>            descPool,

            IOExecutor ioExecutor,
            PoolExecutor poolExecutor     
        );

        future_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config);
    private:
        std::atomic_bool                                     _busy{ false };
        DeviceContext&                                       _deviceContext;
        Queue                                                _copyQueue;
        std::shared_ptr<MTDescriptorAllocator>                      _descPool;
        std::shared_ptr<PipelineCache>                       _pipelineCache;

        std::shared_ptr<SubmissionThreadExecutionContext>    _submissionThreadExecutionContext;
        std::shared_ptr<OneShotCopySubmissionHandler>        _oneShotCopySubmissionHandler;
        IOExecutor                                           _ioExecutor;
        PoolExecutor                                         _poolExecutor;
        std::shared_ptr<HostWriteCombinedRingPool>           _uploadPool;
        CommandPool                                          _uploadCommandPool;

        vkr::CommandBuffer RecordUploadCommands(dynamic_span<ImageStageUploadDesc> imageUploads, const BufferDataUploadTransferDesc& bufferUpload, const PoolSequentialStageZone& stageZone);
    };

}