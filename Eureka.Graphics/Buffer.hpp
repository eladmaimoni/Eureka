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
        AllocatedBufferBase(VmaAllocator allocator);
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
        AllocatedBuffer(VmaAllocator allocator) : AllocatedBufferBase(allocator) {}
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
    
    
    //inline std::atomic_int instances = 0;

    class PoolAllocatedBuffer : public AllocatedBufferBase
    {
        //int _id{};
    protected:
        fu::function<void(void)> _releaseCallback;
        PoolAllocatedBuffer(VmaAllocator allocator, fu::function<void(void)> releaseCallback) 
            : AllocatedBufferBase(allocator), _releaseCallback(std::move(releaseCallback)) 
        {
            //_id = instances.fetch_add(1);
            //DEBUGGER_TRACE("PoolAllocatedBuffer(VmaAllocator allocator, fu::function<void(void)> releaseCallback) {} instances = {}", _id, _id + 1);
        
        }
        PoolAllocatedBuffer(VmaAllocator allocator) : AllocatedBufferBase(allocator) 
        {
            //_id = instances.fetch_add(1);
            //DEBUGGER_TRACE("pool buffer {} instances = {}", _id, _id + 1);
        }
        PoolAllocatedBuffer()
        {
            //_id = instances.fetch_add(1);
            //DEBUGGER_TRACE("pool buffer {} instances = {}", _id, _id + 1);
        }
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
        HostMappedBuffer(VmaAllocator allocator) : BaseBuffer(allocator) {}
        template<typename F>
        HostMappedBuffer(VmaAllocator allocator, F f) : BaseBuffer(allocator, std::move(f)) {}

        ~HostMappedBuffer()
        {
            _ptr = nullptr;
        }
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
            assert(!that._buffer);
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
        // ideal as a stage buffer for uploading large amounts of data to the gpu
        // not ideal when you want to read the data or update it in a random way
    public:
        HostWriteCombinedBuffer(VmaAllocator allocator, const BufferConfig& config);
        EUREKA_DEFAULT_MOVEONLY(HostWriteCombinedBuffer);
    };

    class HostWriteCombinedPoolBuffer : public HostMappedBuffer<PoolAllocatedBuffer>
    {
        friend class HostWriteCombinedRingPool;

        HostWriteCombinedPoolBuffer(
            VmaAllocator    allocator,
            VmaAllocation     allocation,
            vk::Buffer        buffer,
            uint64_t          byteSize,
            void* ptr,
            fu::function<void(void)> releaseCallback
        ) : HostMappedBuffer<PoolAllocatedBuffer>(allocator, std::move(releaseCallback))
            
            //_allocation(allocation),
            //_buffer(buffer),
            //_byteSize(byteSize), ????
            //_ptr(ptr)
        {
            _allocation = allocation;
            _buffer = buffer;
            _byteSize = byteSize;
            _ptr = ptr;
            assert(_releaseCallback);
            assert(_buffer);
        }


    public:
   
        HostWriteCombinedPoolBuffer(HostWriteCombinedPoolBuffer&& that) noexcept
            : HostMappedBuffer<PoolAllocatedBuffer>(std::move(that))
        {
            assert(!that._buffer);
            assert(!that._allocation);
            assert(!that._buffer);
            assert(!that._ptr);
            assert(that._byteSize == 0);
        }

        ~HostWriteCombinedPoolBuffer() noexcept
        {
  
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                  HostVisibleDeviceConstantBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostVisibleDeviceConstantBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
        // AKA BAR memory.
        // this is gpu memory mapped to host virtual address
        // all write are expected to be sequential
        // writes have to travel through PCIe bus
    public:
        HostVisibleDeviceConstantBuffer() = default;
        HostVisibleDeviceConstantBuffer(VmaAllocator allocator, const BufferConfig& config);
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
        VertexAndIndexTransferableDeviceBuffer(VmaAllocator allocator, const BufferConfig& config);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                VertexAndIndexHostVisibleDeviceBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class VertexAndIndexHostVisibleDeviceBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
        // AKA BAR memory.
        // this is gpu memory mapped to host virtual address
        // all write are expected to be sequential
        // writes have to travel through PCIe bus
        
    public:
        VertexAndIndexHostVisibleDeviceBuffer() = default;
        VertexAndIndexHostVisibleDeviceBuffer(VmaAllocator allocator, const BufferConfig& config);
    };
}
