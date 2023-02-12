#include "DescriptorPool.hpp"
#include "Result.hpp"
#include <move.hpp>

namespace eureka::vulkan
{

    DescriptorPool::DescriptorPool(std::shared_ptr<Device> device,
                                   const VkDescriptorPoolCreateInfo& descriptorPoolCreateInfo) :
        _device(std::move(device))
    {
        _pool = _device->CreateDescriptorPool(descriptorPoolCreateInfo);
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& that) noexcept :
        _pool(steal(that._pool)),
        _device(std::move(that._device))
    {}

    DescriptorPool::~DescriptorPool()
    {
        if(_pool)
        {
            _device->DestroyDescriptorPool(_pool);
        }
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& rhs) noexcept
    {
        _pool = steal(rhs._pool);
        _device = std::move(rhs._device);
        return *this;
    }

    VkDescriptorSet DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout& layout)
    {
        VkDescriptorSetAllocateInfo allocInfo {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = _pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
        };

        return _device->AllocateDescriptorSet(allocInfo);
    }

    VkDescriptorSet DescriptorPool::TryAllocateDescriptorSet(const VkDescriptorSetLayout& layout)
    {
        VkDescriptorSetAllocateInfo allocInfo {.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                               .descriptorPool = _pool,
                                               .descriptorSetCount = 1,
                                               .pSetLayouts = &layout};

        return _device->TryAllocateDescriptorSet(allocInfo);
    }

} // namespace eureka::vulkan
