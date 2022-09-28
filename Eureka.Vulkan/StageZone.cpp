#include "StageZone.hpp"

namespace eureka::vulkan
{
    SequentialStageZone::SequentialStageZone(std::shared_ptr<ResourceAllocator>& allocator, uint64_t byteSize) 
        : _buffer(std::move(allocator), byteSize)
    {

    }

    uint64_t SequentialStageZone::Position() const
    {
        return _front;
    }

    uint64_t SequentialStageZone::LeftoverBytes() const
    {
        return _buffer.ByteSize() - _front;
    }

    void SequentialStageZone::Reset()
    {
        _front = 0;
    }

    VkBuffer SequentialStageZone::Buffer() const
    {
        return _buffer.Buffer();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    PoolSequentialStageZone::PoolSequentialStageZone(std::shared_ptr<BufferMemoryPool> pool, uint64_t byteSize) :
        _pool(std::move(pool)),
        _allocation(_pool->AllocateBuffer(byteSize))
    {

    }

    PoolSequentialStageZone::PoolSequentialStageZone(PoolSequentialStageZone&& that)
        : _pool(std::move(that._pool)), _allocation(that._allocation), _front(that._front)
    {
        that._allocation = {};
        that._front = {};
    }

    PoolSequentialStageZone& PoolSequentialStageZone::operator=(PoolSequentialStageZone&& rhs)
    {
        _pool = std::move(rhs._pool);
        _allocation = std::move(rhs._allocation);
        _front = std::move(rhs._front);

        rhs._allocation = {};
        rhs._front = {};
        return *this;
    }

    PoolSequentialStageZone::~PoolSequentialStageZone()
    {
        if (_allocation.allocation)
        {
            _pool->DeallocateBuffer(_allocation);
        }

    }

    uint64_t PoolSequentialStageZone::Position() const
    {
        return _front;
    }

    uint64_t PoolSequentialStageZone::LeftoverBytes() const
    {
        return _allocation.byte_size - _front;
    }

    void PoolSequentialStageZone::Reset()
    {
        _front = 0;
    }

    VkBuffer PoolSequentialStageZone::Buffer() const
    {
        return _allocation.buffer;
    }

}

