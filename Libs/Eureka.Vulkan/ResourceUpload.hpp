#pragma once

#include "../Eureka.Vulkan/Queue.hpp"
#include <containers_aliases.hpp>
#include <cstdint>

namespace eureka::vulkan
{
    struct ImageStageUploadDesc
    {
        dynamic_cspan<uint8_t> unpinned_src_span; // point to data in memory
        uint64_t               stage_zone_offset;
        VkImage                destination_image;
        VkExtent3D             destination_image_extent;
    };

    struct BufferDataUploadTransferDesc
    {
        svec10<dynamic_cspan<uint8_t>> unpinned_src_spans;
        //vk::Buffer                     src_buffer;
        uint64_t                       stage_zone_offset;
        uint64_t                       bytes;
                                       
        VkBuffer                       dst_buffer;
        uint64_t                       dst_offset;
    };

    // CopyQueueSampledImageUploadSequence
    // image upload sequence via a copy queue
    // 1. transition image layout to a layout suitable for receiving the transfer
    // 2. actual transfer
    // 3. release copy queue ownership
    // 4. acquire graphics queue ownership
    struct CopyQueueSampledImageUploadSequence
    {
        VkImageMemoryBarrier copy_queue_pre_transfer_barrier;
        VkBufferImageCopy    copy_queue_transfer;
        VkImageMemoryBarrier copy_queue_release;
        VkImageMemoryBarrier graphics_queue_acquire;
    };

    CopyQueueSampledImageUploadSequence CreateImageUploadCommandSequence(
        const Queue& copyQueue,
        const Queue& graphicsQueue, 
        const ImageStageUploadDesc& imageUploadDesc
    );

    // CopyQueueSampledImageUploadSequence2
    //
    //
    struct CopyQueueSampledImageUploadSequence2
    {
        VkImageMemoryBarrier2         copy_queue_pre_transfer_barrier;
        VkBufferImageCopy2            copy_queue_transfer;
        VkImageMemoryBarrier2         copy_queue_release;
        VkImageMemoryBarrier2         graphics_queue_acquire;
    };

    CopyQueueSampledImageUploadSequence2 CreateImageUploadCommandSequence2(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc
    );
}

