#include "Descriptor.hpp"
#include "DescriptorAllocators.hpp"
#include "move.hpp"
#include <boost/container/small_vector.hpp>
namespace eureka::vulkan
{
    template<typename T> using svec5 = boost::container::small_vector<T, 5>;

    void DescriptorSetBase::SetBinding(VkDescriptorSet set, uint32_t bindingSlot, VkDescriptorType descType, const VkDescriptorBufferInfo& bufferInfo)
    {
        VkWriteDescriptorSet writeDescriptorSet
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = bindingSlot,
            .descriptorCount = 1,
            .descriptorType = descType,
            .pBufferInfo = &bufferInfo
        };

        _device->UpdateDescriptorSet(writeDescriptorSet);
    }

    void DescriptorSetBase::SetBindings(VkDescriptorSet set, uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos)
    {
        svec5<VkWriteDescriptorSet> writes;
        writes.reserve(imageInfos.size());

        for (auto& imgeInfo : imageInfos)
        {
            writes.emplace_back(
                VkWriteDescriptorSet
                {
                    .sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = startSlot,
                    .descriptorCount = 1u,
                    .descriptorType = descType,
                    .pImageInfo = &imgeInfo,
                    .pBufferInfo = nullptr
                }
            );
            ++startSlot;
        }

        _device->UpdateDescriptorSet(writes);
    }

    DescriptorSetBase::DescriptorSetBase(std::shared_ptr<Device> device) : _device(std::move(device))
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////


    DescriptorSetHandle::DescriptorSetHandle(std::shared_ptr<Device> device, VkDescriptorSet set) :
        DescriptorSetBase(std::move(device)),
        _set(set)
    {

    }

    DescriptorSetHandle& DescriptorSetHandle::operator=(DescriptorSetHandle&& rhs) noexcept
    {
        DescriptorSetBase::operator=(std::move(rhs));
        _set = steal(rhs._set);
        return *this;
    }

    DescriptorSetHandle::DescriptorSetHandle(DescriptorSetHandle&& that) noexcept
        : 
        DescriptorSetBase(std::move(that)),
        _set(steal(that._set))
    {

    }

    void DescriptorSetHandle::SetBindings(uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos)
    {
        DescriptorSetBase::SetBindings(_set, startSlot, descType, imageInfos);
    }

    void DescriptorSetHandle::SetBinding(uint32_t bindingSlot, VkDescriptorType descType, const VkDescriptorBufferInfo& bufferInfo)
    {
        DescriptorSetBase::SetBinding(_set, bindingSlot, descType, bufferInfo);
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    FreeableDescriptorSet::FreeableDescriptorSet(
        std::shared_ptr<Device> device, 
        std::shared_ptr<FreeableDescriptorSetAllocator> allocator,
        const VkDescriptorSetLayout& layout
    ) :
        DescriptorSetBase(std::move(device)),
        _allocator(std::move(allocator)),
        _allocation(_allocator->AllocateSet(layout))
    {

    }

    FreeableDescriptorSet::FreeableDescriptorSet(
        std::shared_ptr<Device> device, 
        std::shared_ptr<FreeableDescriptorSetAllocator> allocator
    ) :
        DescriptorSetBase(std::move(device)),
        _allocator(std::move(allocator))
    {

    }

    void FreeableDescriptorSet::Allocate(const VkDescriptorSetLayout& layout)
    {
        Deallocate();
        _allocation = _allocator->AllocateSet(layout);
    }

    void FreeableDescriptorSet::Deallocate()
    {
        if (_allocation.set)
        {
            _allocator->DeallocateSet(_allocation);
        }
    }

    FreeableDescriptorSet::FreeableDescriptorSet(FreeableDescriptorSet&& that) noexcept :
        DescriptorSetBase(std::move(that)),
        _allocator(std::move(that._allocator)),
        _allocation(that._allocation)
    {
        that._allocation = {};
    }



    FreeableDescriptorSet& FreeableDescriptorSet::operator=(FreeableDescriptorSet&& rhs) noexcept
    {
        DescriptorSetBase::operator=(std::move(rhs));
        _allocator = std::move(rhs._allocator);
        _allocation = rhs._allocation;
        rhs._allocation = {};

        return *this;
    }

    FreeableDescriptorSet::~FreeableDescriptorSet()
    {
        Deallocate();
    }

    VkDescriptorSet FreeableDescriptorSet::Get() const
    {
        assert(_allocation.set);
        return _allocation.set;
    }

    void FreeableDescriptorSet::SetBindings(uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos)
    {
        DescriptorSetBase::SetBindings(_allocation.set, startSlot, descType, imageInfos);
    }



}

