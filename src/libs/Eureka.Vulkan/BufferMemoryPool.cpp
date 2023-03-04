#include "BufferMemoryPool.hpp"

namespace eureka::vulkan
{
    BufferMemoryPool::BufferMemoryPool(std::shared_ptr<ResourceAllocator> allocator,
                                       uint64_t                           byteSize,
                                       VmaPoolCreateFlags                 poolFlags,
                                       VkBufferUsageFlags                 usageFlags,
                                       VmaAllocationCreateFlags           allocationFlags) :
        _allocator(std::move(allocator)),
        _allocationFlags(allocationFlags),
        _usageFlags(usageFlags)
    {
        _allocation = _allocator->AllocateBufferPool(byteSize, usageFlags, _allocationFlags, poolFlags);
    }
} // namespace eureka::vulkan