#pragma once
#include <ShadersCache.hpp>
namespace eureka
{
    struct DescriptorSetLayoutCreateInfoBundle
    {
        DescriptorSetLayoutCreateInfoBundle() = default;
        DescriptorSetLayoutCreateInfoBundle(std::size_t bindingCount)
            : bindings(bindingCount)
        {

        }
        uint32_t                                    set_num;
        vk::DescriptorSetLayoutCreateInfo           create_info;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };

    //struct PipelineLayoutCreateInfoBundle
    //{
    //    fixed_capacity_vector<DescriptorSetLayoutCreateInfoBundle> sets;
    //    fixed_capacity_vector<vk::PushConstantRange> push_constants;
    //};

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

