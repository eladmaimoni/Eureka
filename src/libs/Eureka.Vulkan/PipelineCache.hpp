#pragma once

#include "Device.hpp"

namespace eureka::vulkan
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        PipelineCache
    // 
    //////////////////////////////////////////////////////////////////////////
    class PipelineCache
    {
        std::shared_ptr<Device> _device;
        VkPipelineCache _piplineCache{ nullptr };
    public:
        PipelineCache() = default;
        PipelineCache(std::shared_ptr<Device> device);
        ~PipelineCache();
        PipelineCache(const PipelineCache&) = delete;
        PipelineCache& operator=(const PipelineCache&) = delete;
    };
}

