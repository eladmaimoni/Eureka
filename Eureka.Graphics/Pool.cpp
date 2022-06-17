#include "Pool.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{
    HostWriteCombinedRingPool::HostWriteCombinedRingPool(DeviceContext& deviceContext, uint64_t byteSize) : _deviceContext(deviceContext), _allocator(deviceContext.Allocator())
    {
        vk::BufferCreateInfo bufferCreateInfo // unused, just for deducing memory index
        {
            .size = byteSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        uint32_t memTypeIndex;
        VK_CHECK(vmaFindMemoryTypeIndexForBufferInfo(
            _allocator,
            &static_cast<VkBufferCreateInfo&>(bufferCreateInfo),
            &allocationCreateInfo,
            &memTypeIndex
        ));

        VmaPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        poolCreateInfo.memoryTypeIndex = memTypeIndex;
        poolCreateInfo.blockSize = byteSize;
        poolCreateInfo.maxBlockCount = 1;

        VK_CHECK(vmaCreatePool(_allocator, &poolCreateInfo, &_pool));

        _byteSize = byteSize;
    }

    HostWriteCombinedRingPool::~HostWriteCombinedRingPool()
    {
        if (_pool)
        {
            vmaDestroyPool(_allocator, _pool);
        }
    }

    void HostWriteCombinedRingPool::PollPending()
    {
        std::unique_lock lk(_mtx);

        while (!_pending.empty())
        {
            auto pending = std::move(_pending.front());     _pending.pop_front();
            lk.unlock();
            auto allocaton = TryAllocate(pending.byte_size);
            lk.lock();
            if (!allocaton)
            {
                // serve fifo order
                _pending.emplace_front(std::move(pending));
                break; 
            }
            pending.pr.set_result(std::move(*allocaton));
        }
    }

}

