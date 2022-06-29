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

    class DescriptorSetLayout
    {
    protected:
        vkr::DescriptorSetLayout _descriptorSetLayout{ nullptr };
        EUREKA_DEFAULT_MOVEABLE(DescriptorSetLayout);
    public:
        vk::DescriptorSetLayout Get() const { return *_descriptorSetLayout; }
   
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // A single UBO aimed to contain view & projection matrix and perhaps other 
    // per view parameters such as view direction and viewport size
    // 
    //////////////////////////////////////////////////////////////////////////
    class PerViewDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        PerViewDescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(PerViewDescriptorSetLayout);
    };

    class PerNormalMappedModelDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        PerNormalMappedModelDescriptorSetLayout(DeviceContext& deviceContext);
        EUREKA_DEFAULT_MOVEABLE(PerNormalMappedModelDescriptorSetLayout);
    };
}