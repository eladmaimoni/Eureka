#pragma once
#include <ShadersCache.hpp>
#include <DescriptorSetsLayout.hpp>

namespace eureka
{




    struct PipelineLayoutCreateBundle
    {
        PipelineLayoutCreateBundle() = default;
        PipelineLayoutCreateBundle(std::size_t setCount, std::size_t pcblocks)
            : sets_bundle(setCount), push_constants_blocks(pcblocks)
        {

        }
        std::vector<DescriptorSetLayoutCreateInfoBundle> sets_bundle;
        std::vector<vk::PushConstantRange> push_constants_blocks;
    };
    PipelineLayoutCreateBundle ReflectPipeline(dynamic_span<ShaderId> stages);

}

