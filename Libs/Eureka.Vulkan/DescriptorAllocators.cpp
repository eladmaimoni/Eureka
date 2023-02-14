#include "DescriptorAllocators.hpp"
#include "Result.hpp"

namespace eureka::vulkan
{


    FreeableDescriptorSetAllocator::FreeableDescriptorSetAllocator(std::shared_ptr<Device> device, DescriptorAllocatorConfig config /*= {}*/)
        : _device(std::move(device))
    {

    }

    FreeableDescriptorSetAllocator::~FreeableDescriptorSetAllocator()
    {

    }

    DescriptorSetAllocation FreeableDescriptorSetAllocator::AllocateSet(VkDescriptorSetLayout layout)
    {
        for (auto i = 0u; i < _pools.size(); ++i)
        {
            auto& pool = _pools[i];

            auto descSet = pool.TryAllocateDescriptorSet(layout);
        
            if (descSet)
            {
                return DescriptorSetAllocation
                {
                    .set = descSet,
                    .pool = pool.Get()
                };
                
            }
        }

        if (_pools.size() < _config.max_pools)
        {
            _pools.emplace_back(AllocatePool());
            return AllocateSet(layout);
        }

        throw ResultError(VkResult::VK_ERROR_OUT_OF_POOL_MEMORY, "failed allocation");
    }

    void FreeableDescriptorSetAllocator::DeallocateSet(const DescriptorSetAllocation& allocation)
    {
        _device->FreeDescriptorSet(allocation.pool, allocation.set);
    }

    DescriptorPool FreeableDescriptorSetAllocator::AllocatePool()
    {
        std::vector<VkDescriptorPoolSize> perTypeMaxCount(_config.multipliers.size());

        for (auto i = 0u; i < perTypeMaxCount.size(); ++i)
        {
            auto [type, multiplier] = _config.multipliers[i];

            perTypeMaxCount[i] = VkDescriptorPoolSize{ .type = type, .descriptorCount = static_cast<uint32_t>(_config.max_sets_per_pool * multiplier) };
        }

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = _config.max_sets_per_pool,
            .poolSizeCount = static_cast<uint32_t>(perTypeMaxCount.size()),
            .pPoolSizes = perTypeMaxCount.data()
        };

        return DescriptorPool(_device, descriptorPoolCreateInfo);
    }

}

