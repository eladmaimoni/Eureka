#pragma once
#include "DeviceContext.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostWriteCombinedRingPool
    //
    //////////////////////////////////////////////////////////////////////////
    class HostWriteCombinedRingPool
    {
        VmaAllocator  _allocator{ nullptr };
        VmaPool       _pool{ nullptr };
        uint64_t      _byteSize;
    public:
        HostWriteCombinedRingPool(DeviceContext& deviceContext, uint64_t byteSize);
        ~HostWriteCombinedRingPool();
        VmaPool Get() const { return _pool; }    
        uint64_t Size() const { return _byteSize; }

        bool TryAllocate(uint64_t byteSize)
        {
            if (byteSize > _byteSize)
            {
                throw std::invalid_argument("bad");
            }
            

            vk::BufferCreateInfo bufferCreateInfo
            {
                .size = byteSize,
                .usage = vk::BufferUsageFlagBits::eTransferSrc
            };

            VmaAllocationCreateInfo allocationCreateInfo
            {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };


            VmaAllocationInfo allocationInfo{};
            VmaAllocation allocation{};
            vk::Buffer buffer{};
            vk::Result result = static_cast<vk::Result>(vmaCreateBuffer(
                _allocator,
                &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
                &allocationCreateInfo,
                &reinterpret_cast<VkBuffer&>(buffer),
                &allocation,
                &allocationInfo
            ));

            if (result != vk::Result::eSuccess)
            {
                //assert(result == vk::Result::emem)
                return false;
            }
            auto ptr = allocationInfo.pMappedData;
            //auto byteSize = bufferCreateInfo.size;
            assert(ptr);




        }
    };


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
        AllocatedBufferBase& operator=(AllocatedBufferBase&& rhs);
        AllocatedBufferBase(AllocatedBufferBase&& that);
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
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs) = default;
        AllocatedBuffer(AllocatedBuffer&& that) = default;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PoolAllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class PoolAllocatedBuffer : public AllocatedBufferBase
    {
    protected:
        VmaAllocator             _allocator{ nullptr };
        VmaAllocation            _allocation{ nullptr };
        vk::Buffer               _buffer{ nullptr };
        uint64_t                 _byteSize{ 0 };
        fu::function<void(void)> _releaseCallback;

        VmaPool _pool{ nullptr };
        PoolAllocatedBuffer(DeviceContext& deviceContext) : AllocatedBufferBase(deviceContext) {}
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
    //                        HostWriteCombinedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    class HostWriteCombinedBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
    public:
        HostWriteCombinedBuffer(DeviceContext& deviceContext, const BufferConfig& config);
        EUREKA_DEFAULT_MOVEONLY(HostWriteCombinedBuffer);
    };

    class HostWriteCombinedPoolBuffer : public HostMappedBuffer<AllocatedBuffer>
    {
       

        friend class HostWriteCombinedRingPool;
    public:
        ~HostWriteCombinedPoolBuffer()
        {

        }
        HostWriteCombinedPoolBuffer() = default;
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
