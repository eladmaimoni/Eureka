#include "AsyncDataLoader.hpp"
#include "OneShotCopySubmission.hpp"
#include "../Eureka.Vulkan/StageZone.hpp"
#include "../Eureka.Vulkan/Commands.hpp"
#include <profiling.hpp>
#include <debugger_trace.hpp>

namespace eureka::graphics
{

    AsyncDataLoader::AsyncDataLoader(
        std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler, 
        std::shared_ptr<vulkan::BufferMemoryPool> uploadPool
    ) :
        _oneShotSubmissionHandler(std::move(oneShotSubmissionHandler)),
        _uploadPool(std::move(uploadPool))
    {

    }

    AsyncDataLoader::~AsyncDataLoader()
    {

    }

    future_t<void> AsyncDataLoader::UploadImageAsync(const vulkan::ImageStageUploadDesc& transferDesc)
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

        vulkan::PoolSequentialStageZone stageBuffer(_uploadPool, transferDesc.unpinned_src_span.size_bytes());

        stageBuffer.Assign(transferDesc.unpinned_src_span);
  
        co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();

        auto [uploadCommandBuffer, uploadCommandsDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();

        {
            PROFILE_CATEGORIZED_SCOPE("UploadImageAsync commands", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
            vulkan::ScopedCommands commands(uploadCommandBuffer);

            VkDependencyInfo dependencyInfo{};
            dependencyInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.copy_queue_pre_transfer_barrier;

            uploadCommandBuffer.PipelineBarrier(dependencyInfo);
            
            VkCopyBufferToImageInfo2 copyBufferToImageInfo
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
                .srcBuffer = stageBuffer.Buffer(),
                .dstImage = transferDesc.destination_image,
                .dstImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .regionCount = 1,
                .pRegions = &uploadCommandsSequence2.copy_queue_transfer

            };

            uploadCommandBuffer.CopyBufferToImage(copyBufferToImageInfo);

            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.copy_queue_release;

            uploadCommandBuffer.PipelineBarrier(dependencyInfo);
        }


        co_await _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer, uploadCommandsDoneSemaphore);

        auto graphicsQueue = _oneShotSubmissionHandler->GraphicsQueue();

        if (graphicsQueue.Family() != _oneShotSubmissionHandler->CopyQueue().Family())
        {
            VkDependencyInfo dependencyInfo{};
            dependencyInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &uploadCommandsSequence2.graphics_queue_acquire;

            auto [graphicsCommandBuffer, graphicsCommandsDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotGraphicsCommandBuffer();

            //DEBUGGER_TRACE("ZZZ graphicsCommandBuffer = {:#x}", (uint64_t)((VkCommandBuffer)graphicsCommandBuffer));
            {
                vulkan::ScopedCommands sc(graphicsCommandBuffer);

                graphicsCommandBuffer.PipelineBarrier(dependencyInfo);

            }


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
                    .stages = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
                }
            };

            co_await _oneShotSubmissionHandler->AppendGraphicsSubmission(graphicsCommandBuffer, graphicsCommandsDoneSemaphore, waitList);

            //DEBUGGER_TRACE("ZZZ ZONE Graphics Acquire");
        }

        


        co_return;
    }


    //future_t<void> AsyncDataLoader::UploadImagesAndBufferAsync(std::vector<ImageStageUploadDesc> imageTransferDesc, uint64_t totalImageMemory, BufferDataUploadTransferDesc bufferTransferDesc)
    //{
    //    //
    //    // stage buffer allocation
    //    //

    //    auto stageBuffer = co_await _uploadPool->EnqueueAllocation(bufferTransferDesc.bytes + totalImageMemory);

    //    PoolSequentialStageZone stageZone(std::move(stageBuffer));


    //    //
    //    // write to stage buffer
    //    //
    //    for (const auto& imageUploadDesc : imageTransferDesc)
    //    {
    //        stageZone.Assign(imageUploadDesc.unpinned_src_span);
    //    }

    //    for (const auto& srcSpan : bufferTransferDesc.unpinned_src_spans)
    //    {
    //        stageZone.Assign(srcSpan);
    //    }


    //    co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();
    //    
    //    auto [uploadCommandBuffer, uploadDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();

    //    RecordUploadCommands(uploadCommandBuffer, imageTransferDesc, bufferTransferDesc, stageZone);

    //    co_await _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer, uploadDoneSemaphore);


    //    co_return;
    //}

    void AsyncDataLoader::RecordUploadCommands(
        vulkan::LinearCommandBufferHandle uploadCommandBuffer,
        dynamic_span<vulkan::ImageStageUploadDesc> imageUploads,
        const vulkan::BufferDataUploadTransferDesc& bufferUpload,
        const vulkan::PoolSequentialStageZone& stageZone
    )
    {
        PROFILE_CATEGORIZED_SCOPE("Asset Loading Command Recording", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
        DEBUGGER_TRACE("rendering thread fun - recording one shot copy");

       

        auto& copyQueue = _oneShotSubmissionHandler->CopyQueue();
        auto& graphicsQueue = _oneShotSubmissionHandler->GraphicsQueue();

        {
            vulkan::ScopedCommands commands(uploadCommandBuffer);

            svec10<VkImageMemoryBarrier> preTransferImageMemoryBarriers;
            svec10<VkImageMemoryBarrier> postTransferImageMemoryBarriers;
            svec10<VkBufferImageCopy> bufferImageCopies;

            for (const auto& imageUploadDesc : imageUploads)
            {
                auto [preTransferBarrier, bufferImageCopy, copyRelease, graphicsAcquire]
                    = CreateImageUploadCommandSequence(copyQueue, graphicsQueue, imageUploadDesc);

                preTransferImageMemoryBarriers.emplace_back(preTransferBarrier);
                bufferImageCopies.emplace_back(bufferImageCopy);
                postTransferImageMemoryBarriers.emplace_back(copyRelease);
            }

            uploadCommandBuffer.PipelineBarrier(
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr, 0, nullptr,
                static_cast<uint32_t>(preTransferImageMemoryBarriers.size()),
                preTransferImageMemoryBarriers.data()
            );

            for (auto i = 0u; i < imageUploads.size(); ++i)
            {
                uploadCommandBuffer.CopyBufferToImage(
                    stageZone.Buffer(),
                    imageUploads[i].destination_image,
                    VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1u,
                    &bufferImageCopies[i]
                );
            }
            VkBufferCopy bufferCopy{ .srcOffset = bufferUpload.stage_zone_offset, .dstOffset = bufferUpload.dst_offset, .size = bufferUpload.bytes };
            
            uploadCommandBuffer.CopyBuffer(
                stageZone.Buffer(),
                bufferUpload.dst_buffer,
                1u,
                &bufferCopy
            );

            VkBufferMemoryBarrier bufferMemoryBarrier
            {
                 .srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT,
                 .dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
                 .srcQueueFamilyIndex = copyQueue.Family(),
                 .dstQueueFamilyIndex = graphicsQueue.Family(),
                 .buffer = bufferUpload.dst_buffer,
                 .offset = 0,
                 .size = bufferUpload.bytes
            };

            uploadCommandBuffer.PipelineBarrier(
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                0,
                0, nullptr, 1u, &bufferMemoryBarrier,
                static_cast<uint32_t>(postTransferImageMemoryBarriers.size()),
                postTransferImageMemoryBarriers.data()
            );

        }

    }

}

