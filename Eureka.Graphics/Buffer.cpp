#include "Buffer.hpp"
#include <new>

namespace eureka
{



    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    AllocatedBuffer::~AllocatedBuffer()
    {
        if (_buffer)
        {
            vmaDestroyBuffer(_allocator, _buffer, _allocation);
        }
    }

    AllocatedBufferBase::AllocatedBufferBase(VmaAllocator allocator)
        : _allocator(allocator)
    {

    }

    AllocatedBufferBase::AllocatedBufferBase(AllocatedBufferBase&& that) noexcept
        :
        _allocator(that._allocator),
        _allocation(that._allocation),
        _buffer(that._buffer),
        _byteSize(that._byteSize)
    {
        that._allocator = { nullptr };
        that._allocation = { nullptr };
        that._buffer = { nullptr };
        that._byteSize = { 0 };
    }

    AllocatedBufferBase& AllocatedBufferBase::operator=(AllocatedBufferBase&& rhs) noexcept
    {
        if (_buffer)
        {
            vmaDestroyBuffer(_allocator, _buffer, _allocation);
        }

        _buffer = rhs._buffer;
        _allocation = rhs._allocation;
        _allocator = rhs._allocator;
        _byteSize = rhs._byteSize;

        rhs._allocation = nullptr;
        rhs._byteSize = 0;
        rhs._buffer = nullptr;

        return *this;
    }

    uint64_t AllocatedBufferBase::ByteSize() const
    {
        return _byteSize;
    }

    vk::DescriptorBufferInfo AllocatedBufferBase::DescriptorInfo() const
    {
        return vk::DescriptorBufferInfo
        {
            .buffer = _buffer,
            .offset = 0,
            .range = _byteSize
        };
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //                        PoolAllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    PoolAllocatedBuffer::~PoolAllocatedBuffer()
    {
        //auto ii = instances.fetch_sub(1);
        //DEBUGGER_TRACE("PoolAllocatedBuffer::~PoolAllocatedBuffer() buffer id {} live instances = {}", _id, ii - 1);
        if (_buffer)
        { 
            //DEBUGGER_TRACE("PoolAllocatedBuffer::~PoolAllocatedBuffer() deallocate pool buffer {}", _id);
            vmaDestroyBuffer(_allocator, _buffer, _allocation);
            _releaseCallback();
            _buffer = nullptr;
        }
        
    
    }

    PoolAllocatedBuffer::PoolAllocatedBuffer(PoolAllocatedBuffer&& that) noexcept
        : AllocatedBufferBase(std::move(that))/*,*/ 
        //_releaseCallback(std::move(that._releaseCallback))
    {
        _releaseCallback = std::move(that._releaseCallback);
        //_id = instances.fetch_add(1);
        //DEBUGGER_TRACE("PoolAllocatedBuffer(PoolAllocatedBuffer&& that) id {} instances = {}", _id, _id + 1);
        assert(!that._buffer);
    }

    PoolAllocatedBuffer& PoolAllocatedBuffer::PoolAllocatedBuffer::operator=(PoolAllocatedBuffer&& rhs) noexcept
    {

        AllocatedBufferBase::operator=(std::move(rhs));
        _releaseCallback = std::move(rhs._releaseCallback);    
        rhs._releaseCallback = [] {};
        assert(!rhs._buffer);
        return *this;
    }




    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostWriteCombinedBuffer
    //
    //////////////////////////////////////////////////////////////////////////



    HostWriteCombinedBuffer::HostWriteCombinedBuffer(
        VmaAllocator allocator, 
        const BufferConfig& config
    ) 
        : HostMappedBuffer(allocator)
    {

        vk::BufferCreateInfo bufferCreateInfo
        {
            .size = config.byte_size,
            .usage = vk::BufferUsageFlagBits::eTransferSrc
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };


        VmaAllocationInfo allocationInfo{};
     
        VK_CHECK(vmaCreateBuffer(
            _allocator,
            &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
            &allocationCreateInfo,
            &reinterpret_cast<VkBuffer&>(_buffer),
            &_allocation,
            &allocationInfo
        ));

        _ptr = allocationInfo.pMappedData;
        _byteSize = bufferCreateInfo.size;
        assert(_ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                      HostVisibleDeviceConstantBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    HostVisibleDeviceConstantBuffer::HostVisibleDeviceConstantBuffer(VmaAllocator allocator, const BufferConfig& config)
        : HostMappedBuffer(allocator)
    {
        // see 'Advanced data uploading'
        // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html

        vk::BufferCreateInfo bufferCreateInfo
        {
            .size = config.byte_size,
            .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateBuffer(
            _allocator,
            &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
            &allocationCreateInfo,
            &reinterpret_cast<VkBuffer&>(_buffer),
            &_allocation,
            &allocationInfo
        ));

        VkMemoryPropertyFlags memPropFlags{};
        vmaGetAllocationMemoryProperties(_allocator, _allocation, &memPropFlags);

        if (0 == (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            throw std::bad_alloc();
        }

        _ptr = allocationInfo.pMappedData;
        _byteSize = bufferCreateInfo.size;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                VertexAndIndexTransferableDeviceBuffer
    //
    //////////////////////////////////////////////////////////////////////////

    VertexAndIndexTransferableDeviceBuffer::VertexAndIndexTransferableDeviceBuffer(VmaAllocator allocator, const BufferConfig& config) : AllocatedBuffer(allocator)
    {
        vk::BufferCreateInfo bufferCreateInfo
        {
            .size = config.byte_size,
            .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateBuffer(
            _allocator,
            &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
            &allocationCreateInfo,
            &reinterpret_cast<VkBuffer&>(_buffer),
            &_allocation,
            &allocationInfo
        ));

        _byteSize = bufferCreateInfo.size;
    }






    //////////////////////////////////////////////////////////////////////////
    //
    //                VertexAndIndexHostVisibleDeviceBuffer
    //
    //////////////////////////////////////////////////////////////////////////
    VertexAndIndexHostVisibleDeviceBuffer::VertexAndIndexHostVisibleDeviceBuffer(VmaAllocator allocator, const BufferConfig& config)
        : HostMappedBuffer<AllocatedBuffer>(allocator)
    {
        // see 'Advanced data uploading'
        // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html

        vk::BufferCreateInfo bufferCreateInfo
        {
            .size = config.byte_size,
            .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateBuffer(
            _allocator,
            &reinterpret_cast<VkBufferCreateInfo&>(bufferCreateInfo),
            &allocationCreateInfo,
            &reinterpret_cast<VkBuffer&>(_buffer),
            &_allocation,
            &allocationInfo
        ));

        VkMemoryPropertyFlags memPropFlags{};
        vmaGetAllocationMemoryProperties(_allocator, _allocation, &memPropFlags);

        if (0 == (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            throw std::bad_alloc();
        }

        _ptr = allocationInfo.pMappedData;
        _byteSize = bufferCreateInfo.size;
    }

}