#pragma once
#include "Device.hpp"
#include "DescriptorAllocators.hpp"

namespace eureka::vulkan
{
    class FreeableDescriptorSetAllocator;
    struct DescriptorSetAllocation;
    
    class DescriptorSetBase
    {
    protected:
        std::shared_ptr<Device> _device{ nullptr };

        void SetBinding(VkDescriptorSet set, uint32_t bindingSlot, VkDescriptorType descType, const VkDescriptorBufferInfo& bufferInfo);
        void SetBindings(VkDescriptorSet set, uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos);
    
        DescriptorSetBase(std::shared_ptr<Device> device);
    };

    class DescriptorSetHandle final : public DescriptorSetBase
    {
    private:
        VkDescriptorSet _set{ nullptr };

    public:
        VkDescriptorSet Get() { return _set; }
        DescriptorSetHandle(std::shared_ptr<Device> device, VkDescriptorSet set);
        DescriptorSetHandle& operator=(DescriptorSetHandle&& rhs);
        DescriptorSetHandle(DescriptorSetHandle&& that);
        ~DescriptorSetHandle() = default;

        void SetBinding(uint32_t bindingSlot, VkDescriptorType descType, const VkDescriptorBufferInfo& bufferInfo);
        void SetBindings(uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos);
    };


    class FreeableDescriptorSet final : public DescriptorSetBase
    {
        std::shared_ptr<FreeableDescriptorSetAllocator> _allocator;
        DescriptorSetAllocation                         _allocation;
    public:
        FreeableDescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<FreeableDescriptorSetAllocator> allocator);
        FreeableDescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<FreeableDescriptorSetAllocator> allocator, const VkDescriptorSetLayout& layout);
        void Allocate(const VkDescriptorSetLayout& layout);
        void Deallocate();

        void SetBindings(uint32_t startSlot, VkDescriptorType descType, dynamic_cspan<VkDescriptorImageInfo> imageInfos);
        VkDescriptorSet Get() const;
        FreeableDescriptorSet& operator=(FreeableDescriptorSet&& rhs);
        FreeableDescriptorSet(FreeableDescriptorSet&& that);       
        ~FreeableDescriptorSet();



    };

}

