#pragma once
#include "Device.hpp"

namespace eureka::vulkan
{


    class DescriptorSetLayout
    {
    protected:
        std::shared_ptr<Device> _device;
        VkDescriptorSetLayout _layout{ nullptr };
    public:
        DescriptorSetLayout(DescriptorSetLayout&& that) noexcept;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs) noexcept;
        DescriptorSetLayout(const DescriptorSetLayout& that) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout& rhs) = delete;
        ~DescriptorSetLayout();
        DescriptorSetLayout(std::shared_ptr<Device> device, const VkDescriptorSetLayoutCreateInfo& info);
        VkDescriptorSetLayout Get() const { return _layout; }
    };
}

