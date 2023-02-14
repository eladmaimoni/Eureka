#include "ResourceUpload.hpp"

namespace eureka::vulkan
{
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
            // before copying the image to the gpu, we transition the image layout to receive the transfer
            // the access mask theoretically flushes caches for that so that new data will be visible to other ops
            .copy_queue_pre_transfer_barrier = VkImageMemoryBarrier
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE_KHR,
                .dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT, // eMemoryWrite in khronous examples, this one is a bit more specific to the transfer commands
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = copyQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            // copy command 
            // we copy a certain range from a stage buffer
            // this assumes we write a single mip layer and the whole image at once
            .copy_queue_transfer = VkBufferImageCopy
            {
                .bufferOffset = imageUploadDesc.stage_zone_offset,
                .imageSubresource = VkImageSubresourceLayers
                {
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageExtent = imageUploadDesc.destination_image_extent
            },
            // release copy queue ownership and transition layout to shader read only optimal
            // flushes the caches after the transfer 
            //
            .copy_queue_release = VkImageMemoryBarrier
            {
                .srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT, // flush the cache after we are done
                .dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT, // ignored in non unified
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            // acquire ownership via graphics queue
            // invalidate caches before first read
            .graphics_queue_acquire = VkImageMemoryBarrier
            {
                .srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT, // ignored
                .dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT, // invalidate caches before we read for the first time
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = graphicsQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            }
        };
    }


    CopyQueueSampledImageUploadSequence2 CreateImageUploadCommandSequence2(const Queue& copyQueue, const Queue& graphicsQueue, const ImageStageUploadDesc& imageUploadDesc)
    {
        return CopyQueueSampledImageUploadSequence2
        {            
            // Pipeline barrier before the copy to perform a layout transition
            // before copying the image to the gpu, we transition the image layout to receive the transfer
            // the access mask theoretically flushes caches for that so that new data will be visible to other ops
            .copy_queue_pre_transfer_barrier = VkImageMemoryBarrier2
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, // nobody waits for that barrier as this is an initial op
                .srcAccessMask = VK_ACCESS_2_NONE,
                .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT, // eMemoryWrite in khronous examples, this one is a bit more specific to the transfer commands
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),
                .dstQueueFamilyIndex = copyQueue.Family(),
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            // copy command 
            // we copy a certain range from a stage buffer
            // this assumes we write a single mip layer and the whole image at once
            .copy_queue_transfer = VkBufferImageCopy2
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
                .bufferOffset = imageUploadDesc.stage_zone_offset,
                .imageSubresource = VkImageSubresourceLayers
                {
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageExtent = imageUploadDesc.destination_image_extent
            },
            // release copy queue ownership and transition layout to shader read only optimal
            // flushes the caches after the transfer 
            //
            .copy_queue_release = VkImageMemoryBarrier2
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, // ignored in copy queue
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            },
            // acquire ownership via graphics queue
            // invalidate caches before first read
            .graphics_queue_acquire = VkImageMemoryBarrier2
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,  // ignored in graphics queue
                .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                .oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = copyQueue.Family(),  // ignored in unified
                .dstQueueFamilyIndex = graphicsQueue.Family(), // ignored in unified
                .image = imageUploadDesc.destination_image,
                .subresourceRange = VkImageSubresourceRange
                {
                   .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                   .baseMipLevel = 0,
                   .levelCount = 1,
                   .layerCount = 1
                }
            }
        };
    }


}

