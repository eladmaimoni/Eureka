#pragma once
#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"
#include "RenderPass.hpp"
#include "Mesh.hpp"

namespace eureka
{  

    class DescriptorSet
    {
    private:
        vkr::DescriptorSet _set{ nullptr };
    public:
        vk::DescriptorSet Get() { return *_set; }
        DescriptorSet() = default;
        DescriptorSet(vkr::DescriptorSet set)
            :
            _set(std::move(set))
        {

            

        }

        void SetBinding(uint32_t bindingSlot, vk::DescriptorType descType,  const vk::DescriptorBufferInfo& bufferInfo)
        {

            vk::WriteDescriptorSet writeDescriptorSet
            {
                .dstSet = *_set,
                .dstBinding = bindingSlot,
                .descriptorCount = 1,
                .descriptorType = descType,
                .pBufferInfo = &bufferInfo
            
            };

            _set.getDevice().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
        }

    };

    class DescriptorPool
    {
    private:
        std::shared_ptr<vkr::Device> _device;
        vkr::DescriptorPool _pool{ nullptr };

    public:
        DescriptorPool(DeviceContext& deviceContext);
        vk::DescriptorPool Get() const 
        {
            return *_pool;
        }

        vkr::DescriptorSet AllocateSet(vk::DescriptorSetLayout layout)
        {
            vk::DescriptorSetAllocateInfo allocInfo
            {
                .descriptorPool = *_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout

            };
            auto device = **_device;
            vk::DescriptorSet descriptorSet{};

            VK_CHECK(vkAllocateDescriptorSets(
                device,
                (VkDescriptorSetAllocateInfo*)& allocInfo,
                (VkDescriptorSet*)&descriptorSet)
            );

            return vkr::DescriptorSet(*_device, descriptorSet, *_pool);
     
        }
    };



    //////////////////////////////////////////////////////////////////////////
    //
    // This is the set of constant buffers the shader receives
    // the shader expects a single set of descriptors with only one entry ('binding')
    // 
    //////////////////////////////////////////////////////////////////////////
    class PerFrameGeneralPurposeDescriptorSetLayout
    {
        vkr::DescriptorSetLayout _descriptorSetLayout{ nullptr };       
    public:
        vk::DescriptorSetLayout Get() const { return *_descriptorSetLayout; }
        PerFrameGeneralPurposeDescriptorSetLayout() = default;
        PerFrameGeneralPurposeDescriptorSetLayout(PerFrameGeneralPurposeDescriptorSetLayout&& that) = default;
        PerFrameGeneralPurposeDescriptorSetLayout& operator=(PerFrameGeneralPurposeDescriptorSetLayout&& that) = default;
        PerFrameGeneralPurposeDescriptorSetLayout(DeviceContext& deviceContext);
    };



    class ColoredVertexMeshPipeline
    {
        std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> _descriptorSetLayout;
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        vkr::PipelineLayout _pipelineLayout{nullptr};
        vkr::Pipeline       _pipeline{nullptr};
        void Setup(DeviceContext& deviceContext);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> descriptorSetLayout
        );
        ColoredVertexMeshPipeline() = default;
        ColoredVertexMeshPipeline(ColoredVertexMeshPipeline&& that) = default;
        ColoredVertexMeshPipeline& operator=(ColoredVertexMeshPipeline&& rhs) = default;
        vk::PipelineLayout Layout() const;
        vk::Pipeline Get() const;

    };
}
