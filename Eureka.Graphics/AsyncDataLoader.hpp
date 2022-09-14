#pragma once
#include "CommandsUtils.hpp"


namespace eureka
{
    class OneShotSubmissionHandler;
    class HostWriteCombinedRingPool;
    class PoolSequentialStageZone;

    class AsyncDataLoader
    {
    private:
        std::shared_ptr<OneShotSubmissionHandler>  _oneShotSubmissionHandler;
        std::shared_ptr<HostWriteCombinedRingPool> _uploadPool;
    public:
        AsyncDataLoader(
            std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler,
            std::shared_ptr<HostWriteCombinedRingPool> uploadPool
        );
        ~AsyncDataLoader();
        future_t<void> UploadImageAsync(const ImageStageUploadDesc& transferDesc);
        future_t<void> UploadImagesAndBufferAsync(
            std::vector<ImageStageUploadDesc> imageTransferDesc,
            uint64_t totalImageMemory,
            BufferDataUploadTransferDesc bufferTransferDesc
        );

    private:
        void RecordUploadCommands(
            vk::CommandBuffer uploadCommandBuffer,
            dynamic_span<ImageStageUploadDesc> imageUploads,
            const BufferDataUploadTransferDesc& bufferUpload,
            const PoolSequentialStageZone& stageZone
        );
    };



}

