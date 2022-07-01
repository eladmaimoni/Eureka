#pragma once
#include "SubmissionThreadExecutionContext.hpp"
#include "Buffer.hpp"


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
        uint64_t                _position{ 0 };

    public:
        SequentialStageZone(DeviceContext& deviceContext, StageZoneConfig config)
            : _buffer(deviceContext, BufferConfig{ .byte_size = config.bytes_capacity })
        {


        }
        uint64_t Position() const
        {
            return _position;
        }
        uint64_t LeftoverBytes() const
        {
            return _buffer.ByteSize() - _position;
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            _buffer.Assign(s, _position);
            _position += s.size_bytes();
        }

        void Reset()
        {
            _position = 0;
        }

        vk::Buffer Buffer() const
        {
            return _buffer.Buffer();
        }


    };


    class UploadRingBuffer
    {
        UploadRingBuffer(std::shared_ptr<SubmissionThreadExecutionContext> sumbissionContext)
        {

        }
    };

}

