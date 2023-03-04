#pragma once

#include "ResourceAllocator.hpp"
#include <macros.hpp>
#include <span>
#include <assert.hpp>

namespace eureka::vulkan
{
    class AllocatedBufferBase
    {
        // base class for AllocatedBufffer and PoolAllocatedBuffer
    protected:
        std::shared_ptr<ResourceAllocator> _allocator;
        BufferAllocation                 _allocation{ nullptr };
 

        AllocatedBufferBase(std::shared_ptr<ResourceAllocator> allocator);
        AllocatedBufferBase(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize, BufferAllocationPreset preset);
        AllocatedBufferBase() = default;
        ~AllocatedBufferBase() noexcept = default;
        AllocatedBufferBase& operator=(AllocatedBufferBase&& rhs) noexcept;
        AllocatedBufferBase(AllocatedBufferBase&& that) noexcept;
        AllocatedBufferBase& operator=(const AllocatedBufferBase& rhs) = delete;
        AllocatedBufferBase(const AllocatedBufferBase& that) = delete;
    public:
        uint64_t ByteSize() const;
        VkBuffer Buffer() const;
        VkDescriptorBufferInfo DescriptorInfo() const;
    };

    class AllocatedBuffer : public AllocatedBufferBase
    {
    protected:
        AllocatedBuffer(std::shared_ptr<ResourceAllocator> allocator);
        AllocatedBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize, BufferAllocationPreset preset);
        ~AllocatedBuffer() noexcept;
        AllocatedBuffer() = default;
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs) noexcept = default;
        AllocatedBuffer(AllocatedBuffer&& that) noexcept = default;
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
        HostMappedBuffer(std::shared_ptr<ResourceAllocator> allocator) : BaseBuffer(std::move(allocator)) {}
        HostMappedBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize, BufferAllocationPreset preset)
            : BaseBuffer(std::move(allocator), byteSize, preset) {}
        //template<typename F>
        //HostMappedBuffer(std::shared_ptr<ResourceAllocator> allocator, F f) : BaseBuffer(std::move(allocator), std::move(f)) {}

        ~HostMappedBuffer()
        {

        }
        HostMappedBuffer() = default;
        HostMappedBuffer& operator=(HostMappedBuffer&& rhs) noexcept
        {
            BaseBuffer::operator=(std::move(rhs));
            return *this;
        }
        HostMappedBuffer(HostMappedBuffer&& that) noexcept
            : BaseBuffer(std::move(that))
        {
            assert(!that.Buffer());
        }

    public:
        template<typename T>
        T* Ptr()
        {
            assert(BaseBuffer::_allocation.ptr);
            return static_cast<T*>(BaseBuffer::_allocation.ptr);
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s, uint64_t byte_offset = 0)
        {

            assert((s.size_bytes() + byte_offset) <= BaseBuffer::ByteSize());
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                s.data(),
                s.size_bytes()
            );
        }

        template<typename T>
        void Assign(const T& data, uint64_t byte_offset = 0)
        {
            assert((sizeof(T) + byte_offset) <= BaseBuffer::ByteSize());
            std::memcpy(
                Ptr<uint8_t>() + byte_offset,
                &data,
                sizeof(T)
            );
        }

        void InvalidateCachesBeforeHostRead()
        {
            BaseBuffer::_allocator->InvalidateBuffer(BaseBuffer::_allocation);
        }
        void FlushCachesBeforeDeviceRead()
        {
            BaseBuffer::_allocator->FlushBuffer(BaseBuffer::_allocation);
        }
    };



    using HostMappedAllocatedBuffer =  HostMappedBuffer<AllocatedBuffer>;

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
        HostWriteCombinedBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize);
        EUREKA_DEFAULT_MOVEONLY(HostWriteCombinedBuffer);
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
        HostVisibleDeviceConstantBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize);
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
        VertexAndIndexTransferableDeviceBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                VertexAndIndexTransferableDeviceBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostVisibleVertexAndIndexTransferableDeviceBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
    public:
        HostVisibleVertexAndIndexTransferableDeviceBuffer() = default;
        HostVisibleVertexAndIndexTransferableDeviceBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize);
    };


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //
    // 
    //                           Pool Buffers
    //
    // 
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////




    //class PoolAllocatedBuffer : public AllocatedBufferBase
    //{
    //    //int _id{};
    //protected:
    //    PoolAllocatedBuffer(std::shared_ptr<Pool> pool)
    //        : AllocatedBufferBase(allocator)
    //    {
    //        //_id = instances.fetch_add(1);
    //        //DEBUGGER_TRACE("PoolAllocatedBuffer(VmaAllocator allocator, fu::function<void(void)> releaseCallback) {} instances = {}", _id, _id + 1);

    //    }
    //    PoolAllocatedBuffer(VmaAllocator allocator) : AllocatedBufferBase(allocator)
    //    {
    //        //_id = instances.fetch_add(1);
    //        //DEBUGGER_TRACE("pool buffer {} instances = {}", _id, _id + 1);
    //    }
    //    PoolAllocatedBuffer()
    //    {
    //        //_id = instances.fetch_add(1);
    //        //DEBUGGER_TRACE("pool buffer {} instances = {}", _id, _id + 1);
    //    }
    //    ~PoolAllocatedBuffer();
    //    PoolAllocatedBuffer& operator=(PoolAllocatedBuffer&& rhs) noexcept;
    //    PoolAllocatedBuffer(PoolAllocatedBuffer&& that) noexcept;
    //    PoolAllocatedBuffer& operator=(const PoolAllocatedBuffer& rhs) = delete;
    //    PoolAllocatedBuffer(const PoolAllocatedBuffer& that) = delete;
    //};


    //class HostWriteCombinedPoolBuffer : public HostMappedBuffer<PoolAllocatedBuffer>
    //{
    //    friend class HostWriteCombinedRingPool;

    //    HostWriteCombinedPoolBuffer(
    //        VmaAllocator    allocator,
    //        VmaAllocation     allocation,
    //        vk::Buffer        buffer,
    //        uint64_t          byteSize,
    //        void* ptr,
    //        fu::function<void(void)> releaseCallback
    //    ) : HostMappedBuffer<PoolAllocatedBuffer>(allocator, std::move(releaseCallback))

    //        //_allocation(allocation),
    //        //_buffer(buffer),
    //        //_byteSize(byteSize), ????
    //        //_ptr(ptr)
    //    {
    //        _allocation = allocation;
    //        _buffer = buffer;
    //        _byteSize = byteSize;
    //        _ptr = ptr;
    //        assert(_releaseCallback);
    //        assert(_buffer);
    //    }


    //public:

    //    HostWriteCombinedPoolBuffer(HostWriteCombinedPoolBuffer&& that) noexcept
    //        : HostMappedBuffer<PoolAllocatedBuffer>(std::move(that))
    //    {
    //        assert(!that._buffer);
    //        assert(!that._allocation);
    //        assert(!that._buffer);
    //        assert(!that._ptr);
    //        assert(that._byteSize == 0);
    //    }

    //    ~HostWriteCombinedPoolBuffer() noexcept
    //    {

    //    }
    //};

}

