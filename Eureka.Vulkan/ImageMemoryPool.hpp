#include "ResourceAllocator.hpp"

namespace eureka::vulkan
{
    class ImageMemoryPool
    {
        std::shared_ptr<ResourceAllocator> _allocator;
        PoolAllocation                     _allocation;
        VmaAllocationCreateFlags           _allocationFlags;
        Image2DAllocationPreset            _preset;

    public:
        ImageMemoryPool(std::shared_ptr<ResourceAllocator> allocator,
                        uint64_t                           byteSize,
                        VmaPoolCreateFlags                 poolFlags,
                        Image2DAllocationPreset            preset,
                        VmaAllocationCreateFlags           allocationFlags = 0// dedicated or not
                        ) :
            _allocator(std::move(allocator)),
            _allocationFlags(allocationFlags),
            _preset(preset)

        {
            _allocation = _allocator->AllocateImage2DPool(byteSize, preset, _allocationFlags, poolFlags);
        }

        ImageAllocation AllocateImage(const VkExtent2D& extent)
        {
            return _allocator->AllocatePoolImage(_allocation.pool, extent, _preset);
        }
        void DeallocateImage(const ImageAllocation& image)
        {
            return _allocator->DeallocateImage(image);
        }
        ~ImageMemoryPool()
        {
            _allocator->DeallocatePool(_allocation);
        }
    };

} // namespace eureka::vulkan