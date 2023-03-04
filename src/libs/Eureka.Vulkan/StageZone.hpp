#pragma once
#include <containers_aliases.hpp>
#include "../Eureka.Vulkan/Buffer.hpp"
#include "../Eureka.Vulkan/BufferMemoryPool.hpp"


namespace eureka::vulkan
{
    class SequentialStageZone
    {
    private:
        HostWriteCombinedBuffer _buffer;
        uint64_t                _front{ 0 };

    public:
        SequentialStageZone(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize);
        uint64_t Position() const;
        uint64_t LeftoverBytes() const;

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            _buffer.Assign(s, _front);
            _front += s.size_bytes();
        }

        void Reset();
        VkBuffer Buffer() const;
    };

    class PoolSequentialStageZone
    {
    private:
        std::shared_ptr<BufferMemoryPool> _pool;
        BufferAllocation _allocation;
        uint64_t         _front{ 0 };
    public:
        PoolSequentialStageZone(std::shared_ptr<BufferMemoryPool> pool, uint64_t byteSize);
        ~PoolSequentialStageZone();
        PoolSequentialStageZone(const PoolSequentialStageZone&) = delete;
        PoolSequentialStageZone& operator=(const PoolSequentialStageZone&) = delete;
        PoolSequentialStageZone(PoolSequentialStageZone&& that) noexcept;
        PoolSequentialStageZone& operator=(PoolSequentialStageZone&& rhs) noexcept;


        uint64_t Position() const;
        uint64_t LeftoverBytes() const;

        template<typename T>
        void Assign(dcspan<T> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            assert(_allocation.ptr);

            std::memcpy(
                (uint8_t*)_allocation.ptr + Position(),
                (const uint8_t*)s.data(),
                s.size_bytes()
            );
            _front += s.size_bytes();
        }

        void Reset();

        VkBuffer Buffer() const;
    };

}

