#include "CommandsUtils.hpp"

namespace eureka
{


    SampledImageUploadSequence CreateImageUploadCommandSequence2(const Queue& copyQueue, const Queue& graphicsQueue, const ImageStageUploadDesc& imageUploadDesc)
    {
        return SampledImageUploadSequence
        {
            .copy_queue_pre_transfer_barrier = vk::ImageMemoryBarrier2
            {
                .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eCopy,
                .dstAccessMask = vk::AccessFlagBits2::eTransferWrite, // eMemoryWrite in khronous examples, this one is a bit more specific to the transfer commands
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = copyQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            .copy_queue_transfer = vk::BufferImageCopy2
            {
                .bufferOffset = imageUploadDesc.stage_zone_offset,
                .imageSubresource = vk::ImageSubresourceLayers
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageExtent = imageUploadDesc.destination_image_extent
            },
            .copy_queue_release = vk::ImageMemoryBarrier2
            {
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eShaderRead, // ignored in copy queue
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            .graphics_queue_acquire = vk::ImageMemoryBarrier2
            {
                .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
                .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,  // ignored in graphics queue
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eShaderRead, 
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            }
        };
    }

    CopyQueueSampledImageUploadSequence CreateImageUploadCommandSequence(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc)
    {
        // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#upload-data-from-the-cpu-to-an-image-sampled-in-a-fragment-shader
        // https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-queue-transfers
        // https://stackoverflow.com/questions/67993790/how-do-you-properly-transition-the-image-layout-from-transfer-optimal-to-shader

        return CopyQueueSampledImageUploadSequence
        {
            // Pipeline barrier before the copy to perform a layout transition
            .copy_queue_pre_transfer_barrier = vk::ImageMemoryBarrier
            {
                .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
                .dstAccessMask = vk::AccessFlagBits::eTransferWrite, // eMemoryWrite in khronous examples, this one is a bit more specific to the transfer commands
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = copyQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            .copy_queue_transfer = vk::BufferImageCopy
            {
                .bufferOffset = imageUploadDesc.stage_zone_offset,
                .imageSubresource = vk::ImageSubresourceLayers
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageExtent = imageUploadDesc.destination_image_extent
            },
            .copy_queue_release = vk::ImageMemoryBarrier
            {
                .srcAccessMask = vk::AccessFlagBits::eTransferWrite, 
                .dstAccessMask = vk::AccessFlagBits::eShaderRead, // ignored in non unified
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            .graphics_queue_acquire = vk::ImageMemoryBarrier
            {
                .srcAccessMask = vk::AccessFlagBits::eTransferWrite, // ignored
                .dstAccessMask = vk::AccessFlagBits::eShaderRead,
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = graphicsQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = vk::ImageSubresourceRange
                {
                   .aspectMask = vk::ImageAspectFlagBits::eColor,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            }
        };
    }

}

