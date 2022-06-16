#pragma once
#include "DeviceContext.hpp"

namespace eureka
{
    struct BufferConfig
    {
        uint32_t byte_size;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class AllocatedBuffer
    {
    protected:
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
        vk::Buffer        _buffer{ nullptr };
        uint64_t          _byteSize{ 0 };
        AllocatedBuffer(DeviceContext& deviceContext);
    public:
        AllocatedBuffer() = default;    
        virtual ~AllocatedBuffer();
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs);
        AllocatedBuffer& operator=(const AllocatedBuffer& rhs) = delete;
        AllocatedBuffer(AllocatedBuffer&& that);
        AllocatedBuffer(const AllocatedBuffer& that) = delete;
        uint64_t ByteSize() const;
        vk::Buffer Buffer() const { return _buffer; }
        vk::DescriptorBufferInfo DescriptorInfo() const;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostMappedAllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostMappedAllocatedBuffer : public AllocatedBuffer
    {
    protected:
        void* _ptr{ nullptr };
        HostMappedAllocatedBuffer(DeviceContext& deviceContext);
    public:
        virtual ~HostMappedAllocatedBuffer();
        HostMappedAllocatedBuffer() = default;  
        HostMappedAllocatedBuffer& operator=(HostMappedAllocatedBuffer&& rhs);
        HostMappedAllocatedBuffer(HostMappedAllocatedBuffer&& that);

        template<typename T>
        T* Ptr()
        {
            assert(_ptr);
            return static_cast<T*>(_ptr);
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s, uint32_t byte_offset = 0)
        {
            assert((s.size_bytes() + byte_offset) <= _byteSize);
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                s.data(),
                s.size_bytes()
            );
        }

        template<typename T>
        void Assign(const T& data, uint32_t byte_offset = 0)
        {
            assert((sizeof(T) + byte_offset) <= _byteSize);
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                &data,
                sizeof(T)
            );
        }

        void InvalidateCachesBeforeHostRead();
        void FlushCachesBeforeDeviceRead();
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostStageZoneBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostStageZoneBuffer : public HostMappedAllocatedBuffer
    {
    public:
        HostStageZoneBuffer() = default;
        HostStageZoneBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                  HostVisibleDeviceConstantBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostVisibleDeviceConstantBuffer : public HostMappedAllocatedBuffer
    {
    public:
        HostVisibleDeviceConstantBuffer() = default;
        HostVisibleDeviceConstantBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                VertexAndIndexTransferableDeviceBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class VertexAndIndexTransferableDeviceBuffer : public AllocatedBuffer
    {
    public:
        VertexAndIndexTransferableDeviceBuffer() = default;
        VertexAndIndexTransferableDeviceBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };
}
