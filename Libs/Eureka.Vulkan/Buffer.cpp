#include "Buffer.hpp"

namespace eureka::vulkan
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedBufferBase
    //
    //////////////////////////////////////////////////////////////////////////

    VkBuffer AllocatedBufferBase::Buffer() const
    {
        return _allocation.buffer;
    }

    AllocatedBufferBase::AllocatedBufferBase(std::shared_ptr<ResourceAllocator> allocator)
        : _allocator(std::move(allocator))
    {

    }
    
    AllocatedBufferBase::AllocatedBufferBase(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize, BufferAllocationPreset preset) : _allocator(std::move(allocator))
    {
        _allocation = _allocator->AllocateBuffer(byteSize, preset);
    }

    AllocatedBufferBase::AllocatedBufferBase(AllocatedBufferBase&& that) noexcept
        :
        _allocator(std::move(that._allocator)),
        _allocation(that._allocation)
    {
        that._allocation = {};
    }

 

    AllocatedBufferBase& AllocatedBufferBase::operator=(AllocatedBufferBase&& rhs) noexcept
    {
        _allocator = std::move(rhs._allocator);
        _allocation = rhs._allocation;
        rhs._allocation = {};
        return *this;
    }

    uint64_t AllocatedBufferBase::ByteSize() const
    {
        return _allocation.byte_size;
    }

    VkDescriptorBufferInfo AllocatedBufferBase::DescriptorInfo() const
    {
        return VkDescriptorBufferInfo
        {
            .buffer = _allocation.buffer,
            .offset = 0,
            .range = _allocation.byte_size
        };
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedBuffer
    //
    //////////////////////////////////////////////////////////////////////////


    AllocatedBuffer::AllocatedBuffer(std::shared_ptr<ResourceAllocator> allocator) : AllocatedBufferBase(std::move(allocator))
    {

    }

    AllocatedBuffer::AllocatedBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize, BufferAllocationPreset preset)
        : AllocatedBufferBase(std::move(allocator), byteSize, preset)
    {

    }

    AllocatedBuffer::~AllocatedBuffer() noexcept
    {
        if (nullptr != _allocation.buffer)
        {
            _allocator->DeallocateBuffer(_allocation);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                        HostWriteCombinedBuffer
    //
    //////////////////////////////////////////////////////////////////////////
    
    HostWriteCombinedBuffer::HostWriteCombinedBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize)
        : HostMappedBuffer(std::move(allocator), byteSize, BufferAllocationPreset::eHostWriteCombinedBufferAsTransferSrc)
    {

    }

    HostVisibleDeviceConstantBuffer::HostVisibleDeviceConstantBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize)
        : HostMappedBuffer(std::move(allocator), byteSize, BufferAllocationPreset::eHostVisibleDeviceConstantBuffer)
    {
    }

    VertexAndIndexTransferableDeviceBuffer::VertexAndIndexTransferableDeviceBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize)
        : AllocatedBuffer(std::move(allocator), byteSize, BufferAllocationPreset::eVertexAndIndexTransferableDeviceBuffer)
    {

    }

    HostVisibleVertexAndIndexTransferableDeviceBuffer::HostVisibleVertexAndIndexTransferableDeviceBuffer(std::shared_ptr<ResourceAllocator> allocator, uint64_t byteSize)
        : HostMappedBuffer(std::move(allocator), byteSize, BufferAllocationPreset::eHostVisibleVertexAndIndexTransferableDeviceBuffer)
    {

    }

}

