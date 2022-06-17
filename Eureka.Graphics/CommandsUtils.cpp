#include "CommandsUtils.hpp"

namespace eureka
{


    CopyQueueSampledImageUpload MakeCopyQueueSampledImageUpload(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc)
    {
        // https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-queue-transfers
        // https://stackoverflow.com/questions/67993790/how-do-you-properly-transition-the-image-layout-from-transfer-optimal-to-shader

        return CopyQueueSampledImageUpload
        {
            .copy_queue_pre_transfer_barrier = vk::ImageMemoryBarrier
            {
                .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
                .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
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
                .dstAccessMask = vk::AccessFlagBits::eShaderRead, // ignored
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

