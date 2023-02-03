#pragma once
#include <Eureka.Vulkan/ResourceUpload.hpp>
#include <Eureka.Vulkan/Commands.hpp>
#include <future.hpp>
#include <sigslot/signal.hpp>

namespace eureka::vulkan
{
    class PoolSequentialStageZone;
    class BufferMemoryPool;
}

namespace eureka::graphics
{
    class OneShotSubmissionHandler;


    class AsyncDataLoader
    {
    private:
        std::shared_ptr<OneShotSubmissionHandler>  _oneShotSubmissionHandler;
        std::shared_ptr<vulkan::BufferMemoryPool>  _uploadPool;
    public:
        AsyncDataLoader(
            std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler,
            std::shared_ptr<vulkan::BufferMemoryPool> uploadPool
        );
        ~AsyncDataLoader();
        future_t<void> UploadImageAsync(const vulkan::ImageStageUploadDesc& transferDesc);
        //future_t<void> UploadImagesAndBufferAsync(
        //    std::vector<vulkan::ImageStageUploadDesc> imageTransferDesc,
        //    uint64_t totalImageMemory,
        //    vulkan::BufferDataUploadTransferDesc bufferTransferDesc
        //);

    private:
        void RecordUploadCommands(
            vulkan::LinearCommandBufferHandle uploadCommandBuffer,
            dynamic_span<vulkan::ImageStageUploadDesc> imageUploads,
            const vulkan::BufferDataUploadTransferDesc& bufferUpload,
            const vulkan::PoolSequentialStageZone& stageZone
        );
    };



}

