#include "PipelineCache.hpp"

namespace eureka::vulkan
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        PipelineCache
    // 
    //////////////////////////////////////////////////////////////////////////
    PipelineCache::PipelineCache(std::shared_ptr<Device> device) :
        _device(std::move(device)),
        _piplineCache(_device->CreatePipelineCache())
    {

    }

    PipelineCache::~PipelineCache()
    {
        if (_piplineCache)
        {
            _device->DestroyPipelineCache(_piplineCache);
        }
    }

}

