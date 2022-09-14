#include "AsyncDataLoader.hpp"
#include "OneShotCopySubmission.hpp"
#include "Pool.hpp"
#include "UploadRingBuffer.hpp"
#include <profiling.hpp>

namespace eureka
{

    AsyncDataLoader::AsyncDataLoader(
        std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler, 
        std::shared_ptr<HostWriteCombinedRingPool> uploadPool
    ) :
        _oneShotSubmissionHandler(std::move(oneShotSubmissionHandler)),
        _uploadPool(std::move(uploadPool))
    {

    }

    AsyncDataLoader::~AsyncDataLoader()
    {

    }

    future_t<void> AsyncDataLoader::UploadImageAsync(const ImageStageUploadDesc& transferDesc)
    {
        auto uploadCommandsSequence =
            CreateImageUploadCommandSequence(
                _oneShotSubmissionHandler->CopyQueue(),
                _oneShotSubmissionHandler->GraphicsQueue(),
                transferDesc
            );
        auto uploadCommandsSequence2 =
            CreateImageUploadCommandSequence2(
                _oneShotSubmissionHandler->CopyQueue(),
                _oneShotSubmissionHandler->GraphicsQueue(),
                transferDesc
            );

        auto stageBuffer = co_await _uploadPool->EnqueueAllocation(transferDesc.unpinned_src_span.size_bytes());

        stageBuffer.Assign(transferDesc.unpinned_src_span, transferDesc.stage_zone_offset);

        co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();

        auto [uploadCommandBuffer, uploadCommandsDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();

        {
            PROFILE_CATEGORIZED_SCOPE("UploadImageAsync commands", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
            ScopedCommands commands(uploadCommandBuffer);

            vk::DependencyInfo dependencyInfo{};
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.copy_queue_pre_transfer_barrier;

            uploadCommandBuffer.pipelineBarrier2(&dependencyInfo);
            
            vk::CopyBufferToImageInfo2 copyBufferToImageInfo
            {
                .srcBuffer = stageBuffer.Buffer(),
                .dstImage = transferDesc.destination_image,
                .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
                .regionCount = 1,
                .pRegions = &uploadCommandsSequence2.copy_queue_transfer

            };

            uploadCommandBuffer.copyBufferToImage2(copyBufferToImageInfo);

            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.copy_queue_release;

            uploadCommandBuffer.pipelineBarrier2(&dependencyInfo);
        }


        co_await _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer, uploadCommandsDoneSemaphore);

        auto graphicsQueue = _oneShotSubmissionHandler->GraphicsQueue();

        if (graphicsQueue != _oneShotSubmissionHandler->CopyQueue())
        {
            vk::DependencyInfo dependencyInfo{};
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.graphics_queue_acquire;

            auto [graphicsCommandBuffer, graphicsCommandsDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotGraphicsCommandBuffer();

            DEBUGGER_TRACE("ZZZ graphicsCommandBuffer = {:#x}", (uint64_t)((VkCommandBuffer)graphicsCommandBuffer));
            {
                ScopedCommands sc(graphicsCommandBuffer);

                graphicsCommandBuffer.pipelineBarrier2(&dependencyInfo);

            }


            // TODO THERE IS A RACE HERE
            // we need to synchronize the previous queue submit with this one. 
            // 1. the first submit must also attach a signal semaphore (created with its command buffer)
            // 2. the second submit must attach it as a wait semaphore
            // 3. perhaps we should work with timeline semaphores only since it will allow us 
            // to use the semaphore even before we submit it.
            // https://www.khronos.org/blog/vulkan-timeline-semaphores

            std::array< OneShotSubmissionWait, 1> waitList
            {
                OneShotSubmissionWait
                {
                    .semaphore = uploadCommandsDoneSemaphore,
                    .stages = vk::PipelineStageFlagBits::eAllGraphics
                }
            };

            co_await _oneShotSubmissionHandler->AppendGraphicsSubmission(graphicsCommandBuffer, graphicsCommandsDoneSemaphore, waitList);

            //DEBUGGER_TRACE("ZZZ ZONE Graphics Acquire");
        }

        


        co_return;
    }


    future_t<void> AsyncDataLoader::UploadImagesAndBufferAsync(std::vector<ImageStageUploadDesc> imageTransferDesc, uint64_t totalImageMemory, BufferDataUploadTransferDesc bufferTransferDesc)
    {
        //
        // stage buffer allocation
        //

        auto stageBuffer = co_await _uploadPool->EnqueueAllocation(bufferTransferDesc.bytes + totalImageMemory);

        PoolSequentialStageZone stageZone(std::move(stageBuffer));


        //
        // write to stage buffer
        //
        for (const auto& imageUploadDesc : imageTransferDesc)
        {
            stageZone.Assign(imageUploadDesc.unpinned_src_span);
        }

        for (const auto& srcSpan : bufferTransferDesc.unpinned_src_spans)
        {
            stageZone.Assign(srcSpan);
        }


        co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();
        
        auto [uploadCommandBuffer, uploadDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();

        RecordUploadCommands(uploadCommandBuffer, imageTransferDesc, bufferTransferDesc, stageZone);

        co_await _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer, uploadDoneSemaphore);


        co_return;
    }

    void AsyncDataLoader::RecordUploadCommands(vk::CommandBuffer uploadCommandBuffer, dynamic_span<ImageStageUploadDesc> imageUploads, const BufferDataUploadTransferDesc& bufferUpload, const PoolSequentialStageZone& stageZone)
    {
        PROFILE_CATEGORIZED_SCOPE("Asset Loading Command Recording", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
        DEBUGGER_TRACE("rendering thread fun - recording one shot copy");

       

        auto& copyQueue = _oneShotSubmissionHandler->CopyQueue();
        auto& graphicsQueue = _oneShotSubmissionHandler->GraphicsQueue();

        {
            ScopedCommands commands(uploadCommandBuffer);

            svec10<vk::ImageMemoryBarrier> preTransferImageMemoryBarriers;
            svec10<vk::ImageMemoryBarrier> postTransferImageMemoryBarriers;
            svec10<vk::BufferImageCopy> bufferImageCopies;

            for (const auto& imageUploadDesc : imageUploads)
            {
                auto [preTransferBarrier, bufferImageCopy, copyRelease, graphicsAcquire]
                    = CreateImageUploadCommandSequence(copyQueue, graphicsQueue, imageUploadDesc);

                preTransferImageMemoryBarriers.emplace_back(preTransferBarrier);
                bufferImageCopies.emplace_back(bufferImageCopy);
                postTransferImageMemoryBarriers.emplace_back(copyRelease);
            }

            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                {},
                nullptr,
                nullptr,
                preTransferImageMemoryBarriers
            );

            for (auto i = 0u; i < imageUploads.size(); ++i)
            {
                uploadCommandBuffer.copyBufferToImage(
                    stageZone.Buffer(),
                    imageUploads[i].destination_image,
                    vk::ImageLayout::eTransferDstOptimal,
                    { bufferImageCopies[i] }
                );
            }

            uploadCommandBuffer.copyBuffer(
                stageZone.Buffer(),
                bufferUpload.dst_buffer,
                { vk::BufferCopy{.srcOffset = bufferUpload.stage_zone_offset, .dstOffset = bufferUpload.dst_offset, .size = bufferUpload.bytes} }
            );

            vk::BufferMemoryBarrier bufferMemoryBarrier
            {
                 .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
                 .dstAccessMask = vk::AccessFlagBits::eShaderRead,
                 .srcQueueFamilyIndex = copyQueue.Family(),
                 .dstQueueFamilyIndex = graphicsQueue.Family(),
                 .buffer = bufferUpload.dst_buffer,
                 .offset = 0,
                 .size = bufferUpload.bytes
            };

            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eAllCommands,
                {},
                nullptr,
                { bufferMemoryBarrier },
                postTransferImageMemoryBarriers
            );
        }

    }

}

