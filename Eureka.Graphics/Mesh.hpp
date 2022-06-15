#pragma once
#include "DeviceContext.hpp"
#include "vk_error_handling.hpp"
#include "PipelineTypes.hpp"



namespace eureka::mesh
{

    static const std::array<PositionColorVertex, 3> COLORED_TRIANGLE_VERTEX_DATA
    {
        PositionColorVertex{ Eigen::Vector3f{  1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 1.0f, 0.6f, 0.4f } },
        PositionColorVertex{ Eigen::Vector3f{ -1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 0.7f, 1.0f, 0.7f } },
        PositionColorVertex{ Eigen::Vector3f{  0.0f, -1.0f, 0.0f }, Eigen::Vector3f{ 0.96f, 0.72f, 0.96f } }
    };
    //static const std::array<PositionColorVertex, 3> COLORED_TRIANGLE_VERTEX_DATA
    //{
    //    PositionColorVertex{ Eigen::Vector3f{  1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 1.0f, 0.0f, 0.0f } },
    //    PositionColorVertex{ Eigen::Vector3f{ -1.0f,  1.0f, 0.0f }, Eigen::Vector3f{ 0.0f, 1.0f, 0.0f } },
    //    PositionColorVertex{ Eigen::Vector3f{  0.0f, -1.0f, 0.0f }, Eigen::Vector3f{ 0.0f, 0.0f, 1.0f } }
    //};

    static constexpr std::array<uint32_t, 3> COLORED_TRIANGLE_INDEX_DATA = { 0, 1, 2 };

}
namespace eureka
{
    struct BufferConfig 
    {
        uint32_t byte_size;
    };

    class AllocatedBuffer
    {   
    protected:
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
        vk::Buffer        _buffer{ nullptr };
        uint64_t          _byteSize{0};
    public:
        AllocatedBuffer() = default;
        AllocatedBuffer(DeviceContext& deviceContext);
        virtual ~AllocatedBuffer();
        AllocatedBuffer& operator=(AllocatedBuffer&& rhs);
        AllocatedBuffer& operator=(const AllocatedBuffer& rhs) = delete;
        AllocatedBuffer(AllocatedBuffer&& that);
        AllocatedBuffer(const AllocatedBuffer& that) = delete;
        uint64_t ByteSize() const;
        vk::Buffer Buffer() const { return _buffer;}

        vk::DescriptorBufferInfo DescriptorInfo() const
        {
            return vk::DescriptorBufferInfo
            {
                .buffer = _buffer,
                .offset = 0,
                .range = _byteSize
            };
        }
    };


    class HostMappedAllocatedBuffer : public AllocatedBuffer
    {    
    protected:
        void* _ptr{nullptr};
    public:
        virtual ~HostMappedAllocatedBuffer() {}
        HostMappedAllocatedBuffer() = default;
        HostMappedAllocatedBuffer(DeviceContext& deviceContext)
            : AllocatedBuffer(deviceContext)
        {

        }
        HostMappedAllocatedBuffer& operator=(HostMappedAllocatedBuffer&& rhs)
        {
            AllocatedBuffer::operator=(std::move(rhs));
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
            return *this;
        }
        HostMappedAllocatedBuffer(HostMappedAllocatedBuffer&& that)
            : AllocatedBuffer(std::move(that)),
            _ptr(that._ptr)
        {
            that._ptr = nullptr;
        }

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

        void InvalidateCachesBeforeHostRead()
        {
            VK_CHECK(vmaInvalidateAllocation(_allocator, _allocation, 0, _byteSize));
        }

        void FlushCachesBeforeDeviceRead()
        {
            VK_CHECK(vmaFlushAllocation(_allocator, _allocation, 0, _byteSize));
        }
    };

    class HostStageZoneBuffer : public HostMappedAllocatedBuffer
    {
    public:
        HostStageZoneBuffer() = default;
        HostStageZoneBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    class HostVisibleDeviceConstantBuffer : public HostMappedAllocatedBuffer
    {
    public:
        HostVisibleDeviceConstantBuffer() = default;
        HostVisibleDeviceConstantBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    class VertexAndIndexTransferableDeviceBuffer : public AllocatedBuffer
    {
    public:
        VertexAndIndexTransferableDeviceBuffer() = default;
        VertexAndIndexTransferableDeviceBuffer(DeviceContext& deviceContext, const BufferConfig& config);
    };

    class TriangleMesh
    {
     
        void Init()
        {
            static constexpr uint32_t INDICES_BYTE_SIZE = 3 * sizeof(uint32_t);
            static constexpr uint32_t VERTICES_BYTE_SIZE = 3 * sizeof(PositionColorVertex);
            static constexpr uint32_t DATA_BYTE_SIZE = INDICES_BYTE_SIZE + VERTICES_BYTE_SIZE;
        }
        
    
    };
}