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
        eR8G8B8A8UnormSampledShaderResourceRenderTargetTransferSrcDst,
        eD24UnormS8UintDepthImage,
        IMAGE2D_ALLOCATION_PRESETS_COUNT
    };

    class ResourceAllocator
    {
        std::shared_ptr<Instance> _instnace;
        std::shared_ptr<Device>   _device;
        VmaAllocator              _vma {nullptr};

    public:
        ResourceAllocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device);
        ResourceAllocator(const ResourceAllocator&) = delete;
        ResourceAllocator& operator=(const ResourceAllocator&) = delete;
        ~ResourceAllocator();

        BufferAllocation AllocateBuffer(uint64_t byteSize, BufferAllocationPreset preset);
        PoolAllocation   AllocateBufferPool(uint64_t byteSize, PoolAllocationPreset preset);
        ImageAllocation
        AllocateImage2D(const VkExtent2D& extent, Image2DAllocationPreset preset, bool dedicated = false);
        PoolAllocation AllocateBufferPool(uint64_t                 byteSize,
                                          VkBufferUsageFlags       usageFlags,
                                          VmaAllocationCreateFlags allocationFlags,
                                          VmaPoolCreateFlags       poolFlags);

        PoolAllocation AllocateImage2DPool(uint64_t                 byteSize,
                                           Image2DAllocationPreset  preset,
                                           VmaAllocationCreateFlags allocationFlags,
                                           VmaPoolCreateFlags       poolFlags);

        BufferAllocation AllocatePoolBuffer(VmaPool                  pool,
                                            uint64_t                 byteSize,
                                            VkBufferUsageFlags       usage,
                                            VmaAllocationCreateFlags allocationFlags);

        ImageAllocation AllocatePoolImage(VmaPool pool, const VkExtent2D& extent, Image2DAllocationPreset preset);
        //std::optional<BufferAllocation> TryAllocatePoolBuffer(VmaPool pool, uint64_t byteSize);
        void DeallocateBuffer(const BufferAllocation& bufferAllocation);
        void DeallocatePool(const PoolAllocation& poolAllocation);
        void DeallocateImage(const ImageAllocation& imageAllocation);

        void InvalidateBuffer(const BufferAllocation& bufferAllocation);
        void FlushBuffer(const BufferAllocation& bufferAllocation);
    };

    VkImageView CreateImage2DView(const Device& device, VkImage image, Image2DAllocationPreset preset);

} // namespace eureka::vulkan
