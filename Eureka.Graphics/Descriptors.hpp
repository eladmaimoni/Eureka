#pragma once

#include "DeviceContext.hpp"
#include <fixed_capacity_vector.hpp>

namespace eureka
{
    class DescriptorSet
    {
    protected:
        vk::DescriptorSet _set{ nullptr };
        std::shared_ptr<vkr::Device> _device{ nullptr };
    public:
        vk::DescriptorSet Get() { return _set; }
        DescriptorSet() = default;
        virtual ~DescriptorSet() = default;
       
        DescriptorSet& operator=(DescriptorSet&& rhs)
        {
            _set = rhs._set;
            _device = std::move(rhs._device);
            rhs._set = nullptr;
            return *this;
        }
        DescriptorSet(DescriptorSet&& that)
            : _set(that._set),
            _device(std::move(that._device))
        {
            that._set = nullptr;
        }
        DescriptorSet(std::shared_ptr<vkr::Device> device, vk::DescriptorSet set)
            :
            _device(std::move(device)),
            _set(std::move(set))
        {



        }

        void SetBinding(uint32_t bindingSlot, vk::DescriptorType descType, const vk::DescriptorBufferInfo& bufferInfo)
        {

            vk::WriteDescriptorSet writeDescriptorSet
            {
                .dstSet = _set,
                .dstBinding = bindingSlot,
                .descriptorCount = 1,
                .descriptorType = descType,
                .pBufferInfo = &bufferInfo

            };

            _device->updateDescriptorSets({ writeDescriptorSet }, {});
        }

        void SetBindings(uint32_t startSlot, vk::DescriptorType descType, dynamic_cspan<vk::DescriptorImageInfo> imageInfos)
        {
            svec5<vk::WriteDescriptorSet> writes;
            writes.reserve(imageInfos.size());

            for (auto& imgeInfo : imageInfos)
            {
                writes.emplace_back(
                    vk::WriteDescriptorSet
                    {
                        .dstSet = _set,
                        .dstBinding = startSlot,
                        .descriptorCount = 1u,
                        .descriptorType = descType,
                        .pImageInfo = &imgeInfo,
                        .pBufferInfo = nullptr
                    }
                );
                ++startSlot;
            }
   
            _device->updateDescriptorSets(writes, {});
        }
    };

    using DescriptorSetReleaseCallback = std::add_pointer<void(void*, vk::DescriptorPool, vk::DescriptorSet)>::type;

    class FreeableDescriptorSet : public DescriptorSet
    {
        void*                        _allocator{ nullptr };
        vk::DescriptorPool           _pool{ nullptr };
        DescriptorSetReleaseCallback _releaseCallback{nullptr};
    public:
        FreeableDescriptorSet() = default;
        FreeableDescriptorSet(const FreeableDescriptorSet&) = delete;
        FreeableDescriptorSet& operator=(const FreeableDescriptorSet&) = delete;
        FreeableDescriptorSet(FreeableDescriptorSet&& that)
            : 
            DescriptorSet(std::move(that)),
            _allocator(that._allocator),
            _pool(that._pool),
            _releaseCallback(that._releaseCallback)
        {
            that._allocator = nullptr;
            that._pool = nullptr;
            that._releaseCallback = nullptr;
        }
        FreeableDescriptorSet& operator=(FreeableDescriptorSet&& rhs)
        {
            DescriptorSet::operator=(std::move(rhs));
            _allocator = rhs._allocator;
            _pool = rhs._pool;
            _releaseCallback = rhs._releaseCallback;

            rhs._allocator = nullptr;
            rhs._pool = nullptr;
            rhs._releaseCallback = nullptr;
            return *this;
        }
        FreeableDescriptorSet(void* allocator, vk::DescriptorPool pool, DescriptorSetReleaseCallback releaseCallback, std::shared_ptr<vkr::Device> device, vk::DescriptorSet set)
            : 
            DescriptorSet(std::move(device), set),
            _allocator(allocator),
            _releaseCallback(releaseCallback),
            _pool(pool)
        {

        }

        ~FreeableDescriptorSet()
        {
            if (_set)
            {
                _releaseCallback(_allocator, _pool, _set);
            }
        }
    };


    struct DescriptorTypeMultiplier
    {
        vk::DescriptorType type;
        float multiplier;
    };

    const std::vector<DescriptorTypeMultiplier> DEFAULT_DESCRIPTOR_POOL_MULTIPLIERS
    {
        DescriptorTypeMultiplier{ vk::DescriptorType::eSampler, 0.5f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eCombinedImageSampler, 4.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eSampledImage, 4.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eStorageImage, 1.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eUniformTexelBuffer, 1.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eStorageTexelBuffer, 1.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eUniformBuffer, 2.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eStorageBuffer, 2.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eUniformBufferDynamic, 1.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eStorageBufferDynamic, 1.f },
        DescriptorTypeMultiplier{ vk::DescriptorType::eInputAttachment, 0.5f }
    };

    struct MTDescriptorAllocatorConfig
    {
        uint32_t max_pools = 4;
        uint32_t max_sets_per_pool = 500;
        std::vector<DescriptorTypeMultiplier> multipliers = DEFAULT_DESCRIPTOR_POOL_MULTIPLIERS;
    };

    class MTDescriptorAllocator
    {
        // a simple thread safe descriptor allocator
        // allocations are synchronized via a mutex
        // if we find ourselves allocating descriptors on a hot path, than perhaps 
        // it is a better idea to allocate them with a custom pool
        // that is not thread safe and can only be reset at once.
    private:
        MTDescriptorAllocatorConfig _config;
        std::mutex _mtx; // Host access to pools must be externally synchronized
        std::shared_ptr<vkr::Device> _device;
        std::vector<vkr::DescriptorPool> _pools;
        vkr::DescriptorPool AllocatePool();

        void FreeSet(vk::DescriptorPool pool, vk::DescriptorSet set);
    public:
        MTDescriptorAllocator(DeviceContext& deviceContext, MTDescriptorAllocatorConfig config = {});
        FreeableDescriptorSet AllocateSet(vk::DescriptorSetLayout layout);
    
    

    };

 










}