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


    struct ImageUploadTuple
    {
        vk::ImageMemoryBarrier pre_transform_barrier;
        vk::BufferImageCopy    copy;
        vk::ImageMemoryBarrier post_transform_barrier;
    };

    ImageUploadTuple ShaderSampledImageUploadTuple(
        const Queue& copyQueue,
        const Queue& graphicsQueue,
        const ImageStageUploadDesc& imageUploadDesc
    );


}

