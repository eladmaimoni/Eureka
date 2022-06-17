#include "ShaderReflection.hpp"
#include <spirv_reflect.h>
#include <vulkan/vulkan_raii.hpp>

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

    namespace spv
    {
        class ReflectedShaderModule
        {
        public:
            ReflectedShaderModule()
            {
                spvmodule.shader_stage = {};
            }
            ReflectedShaderModule(ReflectedShaderModule&& that)
                : spvmodule(that.spvmodule)
            {
                that.spvmodule.shader_stage = {};
            }
            ReflectedShaderModule&& operator=(ReflectedShaderModule&& rhs)
            {
                if (spvmodule.shader_stage)
                {
                    spvReflectDestroyShaderModule(&spvmodule);
                    spvmodule.source_source = {};
                }
                spvmodule = rhs.spvmodule;
                rhs.spvmodule.shader_stage = {};
            }
            ReflectedShaderModule(ShaderId s)
            {
                SPV_REFLECT_CHECK(spvReflectCreateShaderModule(s.size, s.ptr, &spvmodule));

                assert(spvmodule.shader_stage != 0);
            }
            ~ReflectedShaderModule()
            {
                if (spvmodule.shader_stage != 0)
                {
                    spvReflectDestroyShaderModule(&spvmodule);
                    spvmodule.shader_stage = {};
                }   
            }
            SpvReflectShaderModule* Get() { return &spvmodule; }
        private:
            SpvReflectShaderModule spvmodule;
        };
    }

    PipelineLayoutCreateBundle ReflectPipeline(dynamic_span<ShaderId> stages)
    {
        uint32_t count = 0;

        std::vector<spv::ReflectedShaderModule> reflectedModules;
        
        PipelineLayoutCreateBundle piplineCreateBundle{};
     
        std::vector<DescriptorSetLayoutCreateInfoBundle>& setBundles = piplineCreateBundle.sets_bundle;
        //std::vector<vk::PushConstantRange>& pushBlocksBundles = piplineCreateBundle.push_constants_blocks;

        auto totalSets{ 0 };
        auto totalPushConstantBlocks{ 0 };

        // fount total sets and push constants
        for (auto& s : stages)
        {
            auto& reflectedModule = reflectedModules.emplace_back(s);
         
           
            SPV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(reflectedModule.Get(), &count, NULL));
            totalSets += count;
            SPV_REFLECT_CHECK(spvReflectEnumeratePushConstantBlocks(reflectedModule.Get(), &count, NULL));
            totalPushConstantBlocks += count;
        }

        std::vector<SpvReflectDescriptorSet*> reflectedSets(totalSets);
        PipelineLayoutCreateBundle reflectedPipeline(totalSets, totalPushConstantBlocks);

        for (auto& reflectedModule : reflectedModules)
        {            
            SPV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(reflectedModule.Get(), &count, NULL));
            SPV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(reflectedModule.Get(), &count, reflectedSets.data()));
    

            for (auto i_set = 0u; i_set < count; ++i_set)
            {
                auto reflectedSet = reflectedSets[i_set];
                auto set_num = reflectedSet->set;
                DescriptorSetLayoutCreateInfoBundle* setBundle{ nullptr };
                auto sitr = std::ranges::find_if(
                    setBundles,
                    [set_num](const DescriptorSetLayoutCreateInfoBundle& bundle) { return bundle.set_num == set_num;}
                 );

                if (sitr == setBundles.end())
                {
                    setBundle = &setBundles.emplace_back();
                    setBundle->set_num = set_num;
                }
                else
                {
                    setBundle = &(*sitr);
                }
                
                for (auto i_binding = 0u; i_binding < reflectedSet->binding_count; ++i_binding)
                {
                    const SpvReflectDescriptorBinding& reflectedBinding = *(reflectedSet->bindings[i_binding]);

                    vk::DescriptorSetLayoutBinding layoutBinding{};
                    layoutBinding.binding = reflectedBinding.binding;
                    layoutBinding.descriptorType = static_cast<vk::DescriptorType>(reflectedBinding.descriptor_type);
                    layoutBinding.descriptorCount = 1;
                    layoutBinding.pImmutableSamplers = nullptr;
                    auto bitr = std::ranges::find_if(
                        setBundle->bindings,
                        [&layoutBinding](const vk::DescriptorSetLayoutBinding& existingBinding) 
                        { 
                            return existingBinding == layoutBinding; 
                        }
                    );

                    if (bitr == setBundle->bindings.end())
                    {
                        setBundle->bindings.emplace_back(layoutBinding);
                    } 
                }

            }
        }


        for (auto& setBundle : setBundles)
        {
            setBundle.create_info.bindingCount = static_cast<uint32_t>(setBundle.bindings.size());
            setBundle.create_info.pBindings = setBundle.bindings.data();
        }

        return piplineCreateBundle;
    }
}

