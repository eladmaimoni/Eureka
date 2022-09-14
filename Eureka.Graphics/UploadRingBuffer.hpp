#pragma once
#include "SubmissionThreadExecutionContext.hpp"
#include "Buffer.hpp"
#include "Pool.hpp"

namespace eureka
{
    struct StageZoneConfig
    {
        uint64_t bytes_capacity;
    };

    class SequentialStageZone
    {
    private:
        HostWriteCombinedBuffer _buffer;
        uint64_t                _front{ 0 };

    public:
        SequentialStageZone(DeviceContext& deviceContext, StageZoneConfig config)
            : _buffer(deviceContext.Allocator(), BufferConfig{.byte_size = config.bytes_capacity})
        {


        }
        uint64_t Position() const
        {
            return _front;
        }
        uint64_t LeftoverBytes() const
        {
            return _buffer.ByteSize() - _front;
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            _buffer.Assign(s, _front);
            _front += s.size_bytes();
        }

        void Reset()
        {
            _front = 0;
        }

        vk::Buffer Buffer() const
        {
            return _buffer.Buffer();
        }
    };




    class PoolSequentialStageZone
    {
    private:
        HostWriteCombinedPoolBuffer _buffer;
        uint64_t                _front{ 0 };

    public:
        PoolSequentialStageZone(HostWriteCombinedPoolBuffer&& buffer)
            : _buffer(std::move(buffer))
        {

            //assert(_buffer.Ptr<uint8_t>());
        }


        uint64_t Position() const
        {
            return _front;
        }
        uint64_t LeftoverBytes() const
        {
            return _buffer.ByteSize() - _front;
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            _buffer.Assign(s, _front);
            _front += s.size_bytes();
        }

        void Reset()
        {
            _front = 0;
        }

        vk::Buffer Buffer() const
        {
            return _buffer.Buffer();
        }


    };

}

