#pragma once

#include "DeviceContext.hpp"


namespace eureka
{
    struct ImageStageUploadDesc
    {
        dynamic_cspan<uint8_t> unpinned_src_span;
        uint64_t               stage_zone_offset;
        vk::Image              destination_image;
        vk::Extent3D           destination_image_extent;
    };


    struct CopyQueueSampledImageUpload
    {
        vk::ImageMemoryBarrier copy_queue_pre_transfer_barrier;
        vk::BufferImageCopy    copy_queue_transfer;
        vk::ImageMemoryBarrier copy_queue_release;
        vk::ImageMemoryBarrier graphics_queue_acquire;
    };

    CopyQueueSampledImageUpload MakeCopyQueueSampledImageUpload(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc
    );


}

