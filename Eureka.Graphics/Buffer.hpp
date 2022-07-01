#pragma once
#include "DeviceContext.hpp"

namespace eureka
{
    struct BufferConfig
    {
        uint64_t byte_size;
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
        AllocatedBuffer() = default;    
        ~AllocatedBuffer(); // NOTE: non virtual and protected
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs);
        AllocatedBuffer& operator=(const AllocatedBuffer& rhs) = delete;
        AllocatedBuffer(AllocatedBuffer&& that);
        AllocatedBuffer(const AllocatedBuffer& that) = delete;
    public:
        uint64_t ByteSize() const;
        vk::Buffer Buffer() const { return _buffer; }
        vk::DescriptorBufferInfo DescriptorInfo() const;
    };

    class PoolAllocatedBuffer : public AllocatedBuffer
    {
    protected:
        VmaPool _pool{ nullptr };
        PoolAllocatedBuffer(DeviceContext& deviceContext) : AllocatedBuffer(deviceContext) {}
        PoolAllocatedBuffer() = default;
        ~PoolAllocatedBuffer();

        PoolAllocatedBuffer& operator=(PoolAllocatedBuffer&& rhs);
        PoolAllocatedBuffer(PoolAllocatedBuffer&& that);
        PoolAllocatedBuffer& operator=(const PoolAllocatedBuffer& rhs) = delete;
        PoolAllocatedBuffer(const PoolAllocatedBuffer& that) = delete;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostMappedAllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    template<typename BaseBuffer>
    class HostMappedBuffer : public BaseBuffer
    {
    protected:
        void* _ptr{ nullptr };
        HostMappedBuffer(DeviceContext& deviceContext) : BaseBuffer(deviceContext) {}
        ~HostMappedBuffer() = default; 
        HostMappedBuffer() = default;  
        HostMappedBuffer& operator=(HostMappedBuffer&& rhs)
        {
            BaseBuffer::operator=(std::move(rhs));
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
            return *this;
        }
        HostMappedBuffer(HostMappedBuffer&& that)
            : BaseBuffer(std::move(that)),
            _ptr(that._ptr)
        {
            that._ptr = nullptr;
        }

    public:
        template<typename T>
        T* Ptr()
        {
            assert(_ptr);
            return static_cast<T*>(_ptr);
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s, uint64_t byte_offset = 0)
        {
            assert((s.size_bytes() + byte_offset) <= BaseBuffer::_byteSize);
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                s.data(),
                s.size_bytes()
            );
        }

        template<typename T>
        void Assign(const T& data, uint64_t byte_offset = 0)
        {
            assert((sizeof(T) + byte_offset) <= BaseBuffer::_byteSize);
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                &data,
                sizeof(T)
            );
        }

        void InvalidateCachesBeforeHostRead()
        {
            VK_CHECK(vmaInvalidateAllocation(BaseBuffer::_allocator, BaseBuffer::_allocation, 0, BaseBuffer::_byteSize));
        }
        void FlushCachesBeforeDeviceRead()
        {
            VK_CHECK(vmaFlushAllocation(BaseBuffer::_allocator, BaseBuffer::_allocation, 0, BaseBuffer::_byteSize));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostStageZoneBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostWriteCombinedBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
    public:
        HostWriteCombinedBuffer() = default;
        HostWriteCombinedBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                  HostVisibleDeviceConstantBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostVisibleDeviceConstantBuffer : public HostMappedBuffer<AllocatedBuffer>
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
