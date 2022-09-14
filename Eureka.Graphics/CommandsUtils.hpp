#pragma once

#include "DeviceContext.hpp"


namespace eureka
{
    struct ImageStageUploadDesc
    {
        dynamic_cspan<uint8_t> unpinned_src_span; // point to data in memory
        uint64_t               stage_zone_offset;
        vk::Image              destination_image;
        vk::Extent3D           destination_image_extent;
    };

    struct BufferDataUploadTransferDesc
    {
        svec10<dynamic_cspan<uint8_t>> unpinned_src_spans;
        //vk::Buffer                     src_buffer;
        uint64_t                       stage_zone_offset;
        uint64_t                       bytes;
                                       
        vk::Buffer                     dst_buffer;
        uint64_t                       dst_offset;
    };

    struct CopyQueueSampledImageUploadSequence
    {
        vk::ImageMemoryBarrier copy_queue_pre_transfer_barrier;
        vk::BufferImageCopy    copy_queue_transfer;
        vk::ImageMemoryBarrier copy_queue_release;
        vk::ImageMemoryBarrier graphics_queue_acquire;
    };

    struct SampledImageUploadSequence
    {
        vk::ImageMemoryBarrier2         copy_queue_pre_transfer_barrier;
        vk::BufferImageCopy2            copy_queue_transfer;
        vk::ImageMemoryBarrier2         copy_queue_release;
        vk::ImageMemoryBarrier2         graphics_queue_acquire;
    };

    SampledImageUploadSequence CreateImageUploadCommandSequence2(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc
    );

    CopyQueueSampledImageUploadSequence CreateImageUploadCommandSequence(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc
    );


}

