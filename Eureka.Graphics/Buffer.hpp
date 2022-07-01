#pragma once
#include "DeviceContext.hpp"
#include "vk_error_handling.hpp"

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

    class AllocatedBufferBase
    {
    protected:
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
        vk::Buffer        _buffer{ nullptr };
        uint64_t          _byteSize{ 0 };
        AllocatedBufferBase(DeviceContext& deviceContext);
        AllocatedBufferBase() = default;
        ~AllocatedBufferBase() = default;
        AllocatedBufferBase& operator=(AllocatedBufferBase&& rhs) noexcept;
        AllocatedBufferBase(AllocatedBufferBase&& that) noexcept;
        AllocatedBufferBase& operator=(const AllocatedBufferBase& rhs) = delete;
        AllocatedBufferBase(const AllocatedBufferBase& that) = delete;
    public:
        uint64_t ByteSize() const;
        vk::Buffer Buffer() const { return _buffer; }
        vk::DescriptorBufferInfo DescriptorInfo() const;
    };

    class AllocatedBuffer : public AllocatedBufferBase
    {
    protected:
        AllocatedBuffer(DeviceContext& deviceContext) : AllocatedBufferBase(deviceContext) {}
        ~AllocatedBuffer();
        AllocatedBuffer() = default;
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs) noexcept = default;
        AllocatedBuffer(AllocatedBuffer&& that) noexcept = default;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PoolAllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class PoolAllocatedBuffer : public AllocatedBufferBase
    {
    protected:
        fu::function<void(void)> _releaseCallback;
        PoolAllocatedBuffer(DeviceContext& deviceContext, fu::function<void(void)> releaseCallback) : AllocatedBufferBase(deviceContext), _releaseCallback(std::move(releaseCallback)) {}
        PoolAllocatedBuffer() = default;
        ~PoolAllocatedBuffer();
        PoolAllocatedBuffer& operator=(PoolAllocatedBuffer&& rhs) noexcept;
        PoolAllocatedBuffer(PoolAllocatedBuffer&& that) noexcept;
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

        template<typename ... Args >
        HostMappedBuffer(Args&& ... args) : BaseBuffer(std::forward<Args>(args)...) {}
        ~HostMappedBuffer() = default; 
        HostMappedBuffer() = default;  
        HostMappedBuffer& operator=(HostMappedBuffer&& rhs) noexcept
        {
            BaseBuffer::operator=(std::move(rhs));
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
            return *this;
        }
        HostMappedBuffer(HostMappedBuffer&& that) noexcept
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
    //                        HostWriteCombinedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostWriteCombinedBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
    public:
        HostWriteCombinedBuffer(DeviceContext& deviceContext, const BufferConfig& config);
        EUREKA_DEFAULT_MOVEONLY(HostWriteCombinedBuffer);
    };

    class HostWriteCombinedPoolBuffer : public HostMappedBuffer<PoolAllocatedBuffer>
    {
        friend class HostWriteCombinedRingPool;

        HostWriteCombinedPoolBuffer(
            DeviceContext&    deviceContext,
            VmaAllocation     allocation,
            vk::Buffer        buffer,
            uint64_t          byteSize,
            void* ptr,
            fu::function<void(void)> releaseCallback
        ) : HostMappedBuffer<PoolAllocatedBuffer>(deviceContext,std::move(releaseCallback))
            
            //_allocation(allocation),
            //_buffer(buffer),
            //_byteSize(byteSize), ????
            //_ptr(ptr)
        {
            _allocation = allocation;
            _buffer = buffer;
            _byteSize = byteSize;
            _ptr = ptr;
        }
    public:
        EUREKA_DEFAULT_MOVEONLY(HostWriteCombinedPoolBuffer);
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
