#pragma once
#include "Device.hpp"



namespace eureka::vulkan
{

    class DescriptorPool
    {
        VkDescriptorPool _pool{ nullptr };
        std::shared_ptr<Device> _device;
    public:
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool(std::shared_ptr<Device> device, const VkDescriptorPoolCreateInfo& descriptorPoolCreateInfo);
        DescriptorPool(DescriptorPool&& that);
        DescriptorPool& operator=(DescriptorPool&& rhs);
        ~DescriptorPool();
        VkDescriptorPool Get() const { return _pool; }

        VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout& layout);
        VkDescriptorSet TryAllocateDescriptorSet(const VkDescriptorSetLayout& layout);
    };

}

