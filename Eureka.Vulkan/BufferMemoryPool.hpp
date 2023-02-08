#include "ResourceAllocator.hpp"

namespace eureka::vulkan
{
    class BufferMemoryPool
    {
        std::shared_ptr<ResourceAllocator> _allocator;
        PoolAllocation                     _allocation;
        VmaAllocationCreateFlags           _allocationFlags;
        VkBufferUsageFlags                 _usageFlags;

    public:
        BufferMemoryPool(std::shared_ptr<ResourceAllocator> allocator,
                         uint64_t                           byteSize,
                         VmaPoolCreateFlags                 poolFlags,
                         VkBufferUsageFlags                 usageFlags,
                         VmaAllocationCreateFlags           allocationFlags);

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
} // namespace eureka::vulkan