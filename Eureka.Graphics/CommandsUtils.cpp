#include "CommandsUtils.hpp"

namespace eureka
{


    ImageUploadTuple ShaderSampledImageUploadTuple(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc)
    {
        return ImageUploadTuple
        {
            .pre_transform_barrier = vk::ImageMemoryBarrier
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
            .copy = vk::BufferImageCopy
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
            .post_transform_barrier = vk::ImageMemoryBarrier
            {
                .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
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

