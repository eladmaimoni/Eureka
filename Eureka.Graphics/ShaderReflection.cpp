#include "ShaderReflection.hpp"
#include <spirv_reflect.h>
#include <vulkan/vulkan_raii.hpp>
#include <ShadersCache.hpp>
#include "Descriptors.hpp"

namespace eureka
{
    struct SpvReflectShaderModulePtrDeleter
    {
        void operator()(SpvReflectShaderModule* instance)
        {
            if (instance)
            {
                spvReflectDestroyShaderModule(instance);
            }

        }
    };
    using SpvReflectShaderModulePtr = std::unique_ptr<SpvReflectShaderModule, SpvReflectShaderModulePtrDeleter>;

    void CheckSpvReflect(SpvReflectResult res)
    {
        if (res != SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS)
        {
            throw std::runtime_error("bad");
        }
    }


}

#ifndef SPV_REFLECT_CHECK
#ifdef NDEBUG
#define SPV_REFLECT_CHECK(stmt) eureka::CheckSpvReflect(stmt); 
#else
#define SPV_REFLECT_CHECK(stmt) eureka::CheckSpvReflect(stmt); 
#endif
#endif
namespace eureka
{

    struct DescriptorSetLayoutCreateInfoBundle
    {
        DescriptorSetLayoutCreateInfoBundle() = default;
        DescriptorSetLayoutCreateInfoBundle(std::size_t bindingCount)
            : bindings(bindingCount)
        {

        }
        uint32_t                                              set_num;
        vk::DescriptorSetLayoutCreateInfo                     create_info;
        fixed_capacity_vector<vk::DescriptorSetLayoutBinding> bindings;
    };

    struct PipelineLayoutCreateInfoBundle
    {
        fixed_capacity_vector<DescriptorSetLayoutCreateInfoBundle> sets;
        fixed_capacity_vector<vk::PushConstantRange> push_constants;
    };

    struct ShaderModule
    {
        vkr::ShaderModule shader_module{ nullptr };
        vk::ShaderStageFlagBits type{ };

    };
    struct ReflectedPipeline
    {
        ReflectedPipeline() = default;
        ReflectedPipeline(std::size_t setCount)
            : sets_bundle(setCount)
        {

        }
        fixed_capacity_vector<DescriptorSetLayoutCreateInfoBundle> sets_bundle;
    };


    DescriptorSetLayoutCreateInfoBundle ExtractCreateInfo(SpvReflectShaderModule& spvmodule, const SpvReflectDescriptorSet& reflectedSet)
    {
        DescriptorSetLayoutCreateInfoBundle layoutCreateBundle(reflectedSet.binding_count);
        layoutCreateBundle.bindings.resize(reflectedSet.binding_count);

        for (auto i_binding = 0u; i_binding < reflectedSet.binding_count; ++i_binding)
        {
            const SpvReflectDescriptorBinding& reflectedBinding = *(reflectedSet.bindings[i_binding]);

            vk::DescriptorSetLayoutBinding& layoutBinding = layoutCreateBundle.bindings[i_binding];
            layoutBinding.binding = reflectedBinding.binding;
            layoutBinding.descriptorType = static_cast<vk::DescriptorType>(reflectedBinding.descriptor_type);
            layoutBinding.descriptorCount = 1;

            for (auto i_dim = 0u; i_dim < reflectedBinding.array.dims_count; ++i_dim)
            {
                layoutBinding.descriptorCount *= reflectedBinding.array.dims[i_dim];
            }
            layoutBinding.stageFlags = static_cast<vk::ShaderStageFlagBits>(spvmodule.shader_stage);


        }

        layoutCreateBundle.create_info.bindingCount = reflectedSet.binding_count;
        layoutCreateBundle.create_info.pBindings = layoutCreateBundle.bindings.data();
        layoutCreateBundle.set_num = reflectedSet.set;

        return layoutCreateBundle;
    }

    ReflectedPipeline ReflectPipeline(dynamic_span<ShaderId> stages)
    {
        uint32_t count = 0;

        fixed_capacity_vector<vk::PushConstantRange> pushConstants(stages.size());

        

        for (auto& s : stages)
        {
            SpvReflectShaderModule spvmodule;

            SPV_REFLECT_CHECK(spvReflectCreateShaderModule(s.size, s.ptr, &spvmodule));
            SpvReflectShaderModulePtr{ &spvmodule };


            SPV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL));
            svec5<SpvReflectDescriptorSet*> sets(count);
            SPV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data()));

            fixed_capacity_vector<DescriptorSetLayoutCreateInfoBundle> setsCreateBundle(sets.size());

            for (auto i_set = 0u; i_set < sets.size(); ++i_set)
            {
                const SpvReflectDescriptorSet& reflectedSet = *(sets[i_set]);

                auto layoutCreateBundle = ExtractCreateInfo(spvmodule, reflectedSet);

                setsCreateBundle.emplace_back(std::move(layoutCreateBundle));
            }

            SPV_REFLECT_CHECK(spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL));
            if (count > 1)
            {
                throw std::logic_error("bad");
            }
            if (count == 1)
            {
                SpvReflectBlockVariable* reflectedBlock;
                SPV_REFLECT_CHECK(spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, &reflectedBlock));
                pushConstants.emplace_back(
                    vk::PushConstantRange
                    {
                        .stageFlags = s.shader_type,
                        .offset = reflectedBlock->offset,
                        .size = reflectedBlock->offset
                    });
            }

      

        }
        return ReflectedPipeline
        {

        };



    }
}

