#pragma once

#include "Device.hpp"
#include "Instance.hpp"
#include "vk_mem_alloc.h"
#include <array>

namespace eureka::vulkan
{
    struct PoolAllocation
    {
        VmaPool  pool {nullptr};
        uint64_t byte_size {0};
    };

    struct BufferAllocation
    {
        VmaAllocation allocation {nullptr};
        VkBuffer      buffer {nullptr};
        uint64_t      byte_size {0};
        void*         ptr {nullptr};
    };

    struct ImageAllocation
    {
        VmaAllocation allocation {nullptr};
        VkImage       image {nullptr};
    };

    enum BufferAllocationPreset
    {
        eHostVisibleDeviceConstantBuffer,
        eHostWriteCombinedBufferAsTransferSrc,
        eVertexAndIndexTransferableDeviceBuffer,
        eHostVisibleVertexAndIndexTransferableDeviceBuffer,
        BUFFER_ALLOCATION_PRESETS_COUNT
    };

    enum PoolAllocationPreset
    {
        eHostWriteCombinedLinearPoolAsTransferSrc,
        POOL_ALLOCATION_PRESETS_COUNT
    };

    enum Image2DAllocationPreset
    {
        eR8G8B8A8UnormSampledShaderResource,
        eD24UnormS8UintDepthImage,
        IMAGE2D_ALLOCATION_PRESETS_COUNT
    };

    class ResourceAllocator
    {
        std::shared_ptr<Instance> _instnace;
        std::shared_ptr<Device>   _device;
        VmaAllocator              _vma{ nullptr };

    public:
        ResourceAllocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device);
        ResourceAllocator(const ResourceAllocator&) = delete;
        ResourceAllocator& operator=(const ResourceAllocator&) = delete;
        ~ResourceAllocator();

        BufferAllocation AllocateBuffer(uint64_t byteSize, BufferAllocationPreset preset);
        PoolAllocation   AllocateBufferPool(uint64_t byteSize, PoolAllocationPreset preset);
        ImageAllocation
            AllocateImage2D(const VkExtent2D& extent, Image2DAllocationPreset preset, bool dedicated = false);
        PoolAllocation                  AllocateBufferPool(uint64_t                 byteSize,
            VkBufferUsageFlags       usageFlags,
            VmaAllocationCreateFlags allocationFlags,
            VmaPoolCreateFlags       poolFlags);
        BufferAllocation                AllocatePoolBuffer(VmaPool                  pool,
            uint64_t                 byteSize,
            VkBufferUsageFlags       usage,
            VmaAllocationCreateFlags allocationFlags);
        std::optional<BufferAllocation> TryAllocatePoolBuffer(VmaPool pool, uint64_t byteSize);
        void                            DeallocateBuffer(const BufferAllocation& bufferAllocation);
        void                            DeallocatePool(const PoolAllocation& poolAllocation);
        void                            DeallocateImage(const ImageAllocation& imageAllocation);

        void InvalidateBuffer(const BufferAllocation& bufferAllocation);
        void FlushBuffer(const BufferAllocation& bufferAllocation);
    };

    class BufferMemoryPool
    {
        std::shared_ptr<ResourceAllocator> _allocator;
        PoolAllocation                     _allocation;
        VmaAllocationCreateFlags           _allocationFlags;
        VkBufferUsageFlags                 _usageFlags;

    public:
        BufferMemoryPool(std::shared_ptr<ResourceAllocator> allocator,
                         uint64_t                           byteSize,
                         VkBufferUsageFlags                 usageFlags,
                         VmaAllocationCreateFlags           allocationFlags,
                         VmaPoolCreateFlags                 poolFlags) :
            _allocator(std::move(allocator)),
            _allocationFlags(allocationFlags),
            _usageFlags(usageFlags)
        {
            _allocation = _allocator->AllocateBufferPool(byteSize, usageFlags, _allocationFlags, poolFlags);
        }

        BufferAllocation AllocateBuffer(uint64_t byteSize)
        {
            return _allocator->AllocatePoolBuffer(_allocation.pool, byteSize, _usageFlags, _allocationFlags);
        }
        void DeallocateBuffer(const BufferAllocation& buffer)
        {
            return _allocator->DeallocateBuffer(buffer);
        }
        ~BufferMemoryPool()
        {
            _allocator->DeallocatePool(_allocation);
        }
    };

    VkImageView CreateImage2DView(const Device& device, VkImage image, Image2DAllocationPreset preset);

} // namespace eureka::vulkan
