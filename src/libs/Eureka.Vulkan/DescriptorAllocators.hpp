#pragma once
#include "Device.hpp"
#include "DescriptorPool.hpp"
#include <mutex>

namespace eureka::vulkan
{
    struct DescriptorTypeMultiplier
    {
        VkDescriptorType type;
        float multiplier;
    };

    const std::vector<DescriptorTypeMultiplier> DEFAULT_DESCRIPTOR_POOL_MULTIPLIERS
    {
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
        DescriptorTypeMultiplier{ VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
    };

    struct DescriptorAllocatorConfig
    {
        uint32_t max_pools = 4;
        uint32_t max_sets_per_pool = 500;
        std::vector<DescriptorTypeMultiplier> multipliers = DEFAULT_DESCRIPTOR_POOL_MULTIPLIERS;
    };

    struct DescriptorSetAllocation
    {
        VkDescriptorSet set{ nullptr };
        VkDescriptorPool pool{ nullptr };
    };

    class FreeableDescriptorSetAllocator
    {
        // a simple descriptor allocator.
        // - each allocated descriptor set must be deallocated individually (instead of pool reset).
        // - not thread safe - allocations must be externally synchronized
        // - suitable for allocations that live for a long time - static objects.
        // - if we find ourselves allocating descriptors on a hot path, than perhaps 
        //   it is a better idea to allocate them with a custom pool
        //   that can  be reset at once.
    private:
        DescriptorAllocatorConfig _config;
        std::shared_ptr<Device> _device;
        std::vector<DescriptorPool> _pools;
        DescriptorPool AllocatePool();
    public:
        FreeableDescriptorSetAllocator(std::shared_ptr<Device> device, DescriptorAllocatorConfig config = {});
        ~FreeableDescriptorSetAllocator();
        DescriptorSetAllocation AllocateSet(VkDescriptorSetLayout layout);
        void DeallocateSet(const DescriptorSetAllocation& allocation);


    };
}

