#include "Descriptors.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{
    // TODO: these numbers should be much higher and possibly deduces from the scene
    // roughly - the number of sets should be at least the number of different 
    // models we draw as every model usually requires a different set of textures
    inline constexpr uint32_t DEFAULT_MAX_DESCRIPTOR_SETS = 20;
    inline constexpr uint32_t DEFAULT_MAX_UBO_DESCRIPTORS = 3;
    inline constexpr uint32_t DEFAULT_MAX_COMBINED_IMAGE_SAMPLERS_DESCRIPTORS = 50;


    vkr::DescriptorPool MTDescriptorAllocator::AllocatePool()
    {

        std::vector<vk::DescriptorPoolSize> perTypeMaxCount(_config.multipliers.size());

        for (auto i = 0u; i < perTypeMaxCount.size(); ++i)
        {
            auto [type, multiplier] = _config.multipliers[i];

            perTypeMaxCount[i] = vk::DescriptorPoolSize{ .type = type, .descriptorCount = static_cast<uint32_t>(_config.max_sets_per_pool * multiplier) };
        }

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo
        {
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = _config.max_sets_per_pool,
            .poolSizeCount = static_cast<uint32_t>(perTypeMaxCount.size()),
            .pPoolSizes = perTypeMaxCount.data()
        };

        return _device->createDescriptorPool(descriptorPoolCreateInfo);
    }

    void MTDescriptorAllocator::FreeSet(vk::DescriptorPool pool, vk::DescriptorSet set)
    {
        VK_CHECK(vkFreeDescriptorSets(**_device, pool,1, reinterpret_cast<VkDescriptorSet*>(&set)));
    }



    MTDescriptorAllocator::MTDescriptorAllocator(DeviceContext& deviceContext, MTDescriptorAllocatorConfig config)
        :
        _config(std::move(config)),
        _device(deviceContext.LogicalDevice())
    {
        _pools.emplace_back(AllocatePool());
    }


    FreeableDescriptorSet MTDescriptorAllocator::AllocateSet(vk::DescriptorSetLayout layout)
    {
        vk::DescriptorSet descriptorSet{};
        auto device = **_device;

        std::scoped_lock lk(_mtx);
        //auto poolCount = _pools.size();

        vk::Result result{};
        for (auto i = 0u; i < _pools.size(); ++i)
        {
            auto pool = *_pools[i];

            vk::DescriptorSetAllocateInfo allocInfo
            {
                .descriptorPool = pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout
            };

            result = static_cast<vk::Result>(vkAllocateDescriptorSets(
                device,
                (VkDescriptorSetAllocateInfo*)&allocInfo,
                (VkDescriptorSet*)&descriptorSet
            ));

            if (result == vk::Result::eSuccess)
            {
                return FreeableDescriptorSet(
                    this,
                    pool,
                    [](void* self, vk::DescriptorPool pool, vk::DescriptorSet set)
                    {
                        auto _this = static_cast<MTDescriptorAllocator*>(self);
                        _this->FreeSet(pool, set);
                    },
                    _device,
                    descriptorSet
                );
            }
            else if (result == vk::Result::eErrorFragmentedPool || result == vk::Result::eErrorOutOfPoolMemory)
            {
                if (i < (_pools.size() - 1))
                {
                    continue; // try next pool
                }
                else if (_pools.size() < _config.max_pools)
                {
                    _pools.emplace_back(AllocatePool());
                    continue; // new pool is allocated
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        vk::throwResultException(result, "allocation failed");
    }


}
