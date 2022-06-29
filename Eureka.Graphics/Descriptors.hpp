#pragma once

#include "DeviceContext.hpp"

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

        void SetBinding(uint32_t bindingSlot, vk::DescriptorType descType, const vk::DescriptorBufferInfo& bufferInfo)
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

        vkr::DescriptorSet AllocateSet(vk::DescriptorSetLayout layout);
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
}