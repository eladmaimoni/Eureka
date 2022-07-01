#pragma once

#include <DeviceContext.hpp>
#include <GraphicsDefaults.hpp>
#include <SecondaryCommandRecorder.hpp>
#include <Commands.hpp>
#include <SubmissionThreadExecutionContext.hpp>
#include <OneShotCopySubmission.hpp>
#include <UploadRingBuffer.hpp>

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

    struct Image2DUploadTransferDesc
    {
        dynamic_cspan<uint8_t> src_span;
        uint64_t    stage_zone_offset;
        vk::Image   destination_image;
        vk::Extent3D destination_image_extent;
    };

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

    inline constexpr uint64_t STAGE_ZONE_SIZE = 1024 * 1024 * 400; // 20MB

    class AssetLoader
    {
    public:
        AssetLoader(
            DeviceContext& deviceContext,
            Queue queue,
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
            std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
            IOExecutor ioExecutor,
            PoolExecutor poolExecutor     
        );

        result_t<LoadedModel> LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config);



    private:
        std::atomic_bool                                     _busy{ false };
        DeviceContext&                                       _deviceContext;
        Queue                                                _copyQueue;
        std::shared_ptr<SubmissionThreadExecutionContext>    _submissionThreadExecutionContext;
        std::shared_ptr<OneShotCopySubmissionHandler>        _oneShotCopySubmissionHandler;
        IOExecutor                                           _ioExecutor;
        PoolExecutor                                         _poolExecutor;
        SequentialStageZone                                  _stageZone;
        CommandPool                                          _uploadCommandPool;

        vkr::CommandBuffer RecordUploadCommands(dynamic_span<Image2DUploadTransferDesc> imageUploads, const BufferDataUploadTransferDesc& bufferUpload);
    };

}