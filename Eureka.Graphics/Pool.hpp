#pragma once
#include "Buffer.hpp"

namespace eureka
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostWriteCombinedRingPool
    //
    //////////////////////////////////////////////////////////////////////////
    class HostWriteCombinedRingPool
    {
        DeviceContext& _deviceContext;
        VmaAllocator  _allocator{ nullptr };
        VmaPool       _pool{ nullptr };
        uint64_t      _byteSize;
    public:
        HostWriteCombinedRingPool(DeviceContext& deviceContext, uint64_t byteSize);
        ~HostWriteCombinedRingPool();
        VmaPool Get() const { return _pool; }
        uint64_t Size() const { return _byteSize; }

        std::optional<HostWriteCombinedPoolBuffer> TryAllocate(uint64_t byteSize)
        {
            if (byteSize > _byteSize)
            {
                throw std::invalid_argument("bad");
            }
            
            vk::BufferCreateInfo bufferCreateInfo
            {
                .size = byteSize,
                .usage = vk::BufferUsageFlagBits::eTransferSrc
            };

            VmaAllocationCreateInfo allocationCreateInfo
            {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };


            VmaAllocationInfo allocationInfo{};
            VmaAllocation allocation{};
            vk::Buffer buffer{};
            vk::Result result = static_cast<vk::Result>(vmaCreateBuffer(
                _allocator,
                &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
                &allocationCreateInfo,
                &reinterpret_cast<VkBuffer&>(buffer),
                &allocation,
                &allocationInfo
            ));

            if (result != vk::Result::eSuccess)
            {
                //assert(result == vk::Result::emem)
                return std::nullopt;
            }

            assert(allocationInfo.pMappedData);

            return HostWriteCombinedPoolBuffer(
                _deviceContext,
                allocation,
                buffer,
                bufferCreateInfo.size,
                allocationInfo.pMappedData,
                []() {}
            );

        }
    };

}

