#include "ResourceAllocator.hpp"
#include <compiler.hpp>
#include "Result.hpp"

EUREKA_MSVC_WARNING_PUSH_DISABLE(4100 4127 4189 4505 4324)
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
EUREKA_MSVC_WARNING_POP

namespace eureka::vulkan
{
    struct BufferAllocationPresetVals
    {
        constexpr BufferAllocationPresetVals(VkBufferUsageFlags buffer_usage_bits, VmaAllocationCreateFlags allocation_flags)
            : buffer_usage_bits(buffer_usage_bits), allocation_flags(allocation_flags) { }
        VkBufferUsageFlags buffer_usage_bits;
        VmaAllocationCreateFlags allocation_flags;
    };

    struct BufferPoolAllocationPresetVals
    {
        constexpr BufferPoolAllocationPresetVals(VkBufferUsageFlags buffer_usage_bits, VmaAllocationCreateFlags allocation_flags, VmaPoolCreateFlags pool_flags)
            : buffer_usage_flags(buffer_usage_bits), allocation_flags(allocation_flags), pool_flags(pool_flags) { }
        VkBufferUsageFlags       buffer_usage_flags;
        VmaAllocationCreateFlags allocation_flags;
        VmaPoolCreateFlags       pool_flags;
    };

    struct Image2DAllocationPresetVals
    {
        constexpr Image2DAllocationPresetVals(VkFormat format, uint32_t mip_levels, VkSampleCountFlagBits samples, VkImageUsageFlags usage_flags, VkImageAspectFlags aspect_flags)
            : format(format), mip_levels(mip_levels), samples(samples), usage_flags(usage_flags), aspect_flags(aspect_flags) { }

        VkFormat format;
        uint32_t mip_levels = 1;
        VkSampleCountFlagBits samples;
        VkImageUsageFlags usage_flags;
        VkImageAspectFlags aspect_flags;
    };

    inline constexpr std::array<BufferAllocationPresetVals, BufferAllocationPreset::BUFFER_ALLOCATION_PRESETS_COUNT> BUFFER_ALLOCATION_PRESETS
    {
        // eHostVisibleDeviceConstantBuffer
        BufferAllocationPresetVals(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT),
        // eHostWriteCombinedBufferAsTransferSrc
        BufferAllocationPresetVals(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT),
        // eVertexAndIndexTransferableDeviceBuffer
        BufferAllocationPresetVals(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0),
        // eHostVisibleVertexAndIndexTransferableDeviceBuffer
        BufferAllocationPresetVals(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
    };

    inline constexpr std::array<BufferPoolAllocationPresetVals, PoolAllocationPreset::POOL_ALLOCATION_PRESETS_COUNT> POOL_ALLOCATION_PRESETS
    {
        // eHostWriteCombinedLinearPoolAsTransferSrc
        BufferPoolAllocationPresetVals(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT)
    };

    inline constexpr std::array<Image2DAllocationPresetVals, Image2DAllocationPreset::IMAGE2D_ALLOCATION_PRESETS_COUNT> IMAGE2D_ALLOCATION_PRESETS
    {
        // eR8G8B8A8UnormSampledShaderResource
        Image2DAllocationPresetVals(VkFormat::VK_FORMAT_R8G8B8A8_UNORM, 1, VK_SAMPLE_COUNT_1_BIT , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT),
        // eR8G8B8A8UnormSampledShaderResourceRenderTargetTransferSrcDst
        Image2DAllocationPresetVals(VkFormat::VK_FORMAT_R8G8B8A8_UNORM, 1, VK_SAMPLE_COUNT_1_BIT , VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT),
        // eD24UnormS8UintDepthImage
        Image2DAllocationPresetVals(VkFormat::VK_FORMAT_D24_UNORM_S8_UINT, 1, VK_SAMPLE_COUNT_1_BIT , VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT)
    };


    ResourceAllocator::ResourceAllocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device) :
        _instnace(std::move(instance)),
        _device(std::move(device))
    {
        VmaVulkanFunctions vulkanFunction{};

        vulkanFunction.vkAllocateMemory = vkAllocateMemory;
        vulkanFunction.vkBindBufferMemory = vkBindBufferMemory;
        vulkanFunction.vkBindImageMemory = vkBindImageMemory;
        vulkanFunction.vkCreateBuffer = vkCreateBuffer;
        vulkanFunction.vkCreateImage = vkCreateImage;
        vulkanFunction.vkDestroyBuffer = vkDestroyBuffer;
        vulkanFunction.vkDestroyImage = vkDestroyImage;
        vulkanFunction.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vulkanFunction.vkFreeMemory = vkFreeMemory;
        vulkanFunction.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vulkanFunction.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vulkanFunction.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vulkanFunction.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vulkanFunction.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vulkanFunction.vkMapMemory = vkMapMemory;
        vulkanFunction.vkUnmapMemory = vkUnmapMemory;
        vulkanFunction.vkCmdCopyBuffer = vkCmdCopyBuffer;
        vulkanFunction.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vulkanFunction.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo
        {
            .flags = 0,
            .physicalDevice = _device->GetPhysicalDevice(),
            .device = _device->GetDevice(),
            .pVulkanFunctions = &vulkanFunction,
            .instance = _instnace->Get(),
            .vulkanApiVersion = _device->GetApiVersion().Get()
        };
        /*
        VMA_ASSERT(m_VulkanFunctions.vkGetInstanceProcAddr && m_VulkanFunctions.vkGetDeviceProcAddr &&
        "To use VMA_DYNAMIC_VULKAN_FUNCTIONS in new versions of VMA you now have to pass "
        "VmaVulkanFunctions::vkGetInstanceProcAddr and vkGetDeviceProcAddr as VmaAllocatorCreateInfo::pVulkanFunctions. "
        "Other members can be null.");
        
        */
        VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &_vma));
    }

    ResourceAllocator::~ResourceAllocator()
    {
        if (_vma)
        {
            vmaDestroyAllocator(_vma);
        }
    }

    BufferAllocation ResourceAllocator::AllocateBuffer(uint64_t byteSize, BufferAllocationPreset preset)
    {
        VkBufferCreateInfo bufferCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = byteSize,
            .usage = BUFFER_ALLOCATION_PRESETS[preset].buffer_usage_bits
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = BUFFER_ALLOCATION_PRESETS[preset].allocation_flags,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        BufferAllocation mappedAllocation{};
        VmaAllocationInfo allocationInfo{};
        VK_CHECK(vmaCreateBuffer(
            _vma,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &mappedAllocation.buffer,
            &mappedAllocation.allocation,
            &allocationInfo
        ));

        assert(allocationInfo.size == byteSize);
        mappedAllocation.ptr = allocationInfo.pMappedData;
        mappedAllocation.byte_size = allocationInfo.size;
        return mappedAllocation;
    }
    
    PoolAllocation ResourceAllocator::AllocateBufferPool(uint64_t byteSize, PoolAllocationPreset preset)
    {
        return AllocateBufferPool(
            byteSize,
            POOL_ALLOCATION_PRESETS[preset].buffer_usage_flags,
            POOL_ALLOCATION_PRESETS[preset].allocation_flags,
            POOL_ALLOCATION_PRESETS[preset].pool_flags
        );
    }

    PoolAllocation ResourceAllocator::AllocateBufferPool(uint64_t byteSize, VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags allocationFlags, VmaPoolCreateFlags poolFlags)
    {
        VkBufferCreateInfo bufferCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = byteSize,
            .usage = usageFlags
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = allocationFlags,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        uint32_t memTypeIndex;
        VK_CHECK(vmaFindMemoryTypeIndexForBufferInfo(
            _vma,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &memTypeIndex
        ));

        VmaPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.flags = poolFlags;
        poolCreateInfo.memoryTypeIndex = memTypeIndex;
        poolCreateInfo.blockSize = byteSize;
        poolCreateInfo.maxBlockCount = 1;

        PoolAllocation poolAllocation{};

        VK_CHECK(vmaCreatePool(_vma, &poolCreateInfo, &poolAllocation.pool));
        assert(poolCreateInfo.blockSize == byteSize);

        poolAllocation.byte_size = poolCreateInfo.blockSize;
        return poolAllocation;
    }

    PoolAllocation ResourceAllocator::AllocateImage2DPool(uint64_t byteSize, Image2DAllocationPreset preset, VmaAllocationCreateFlags allocationFlags, VmaPoolCreateFlags poolFlags)
    {
        const Image2DAllocationPresetVals& presetVals = IMAGE2D_ALLOCATION_PRESETS[preset];


        VkImageCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VkImageType::VK_IMAGE_TYPE_2D,
            .format = presetVals.format,
            .extent = VkExtent3D{1920, 1080, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
            .usage = presetVals.usage_flags,
            .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
        };


        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = allocationFlags,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        uint32_t memTypeIndex;
        VK_CHECK(vmaFindMemoryTypeIndexForImageInfo(
            _vma,
            &createInfo,
            &allocationCreateInfo,
            &memTypeIndex
        ));

        VmaPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.flags = poolFlags;
        poolCreateInfo.memoryTypeIndex = memTypeIndex;
        poolCreateInfo.blockSize = byteSize;
        poolCreateInfo.maxBlockCount = 0;

        PoolAllocation poolAllocation{};

        VK_CHECK(vmaCreatePool(_vma, &poolCreateInfo, &poolAllocation.pool));
        assert(poolCreateInfo.blockSize == byteSize);

        poolAllocation.byte_size = poolCreateInfo.blockSize;

        return poolAllocation;
    }



    BufferAllocation ResourceAllocator::AllocatePoolBuffer(VmaPool pool, uint64_t byteSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags allocationFlags)
    {
        VkBufferCreateInfo bufferCreateInfo
        {
            // NOTE: no usage since this buffer is from a pool
            .sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = byteSize,
            .usage = usage,
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .flags = allocationFlags,
            .pool = pool 
        };

        VmaAllocationInfo allocationInfo{};
   
        BufferAllocation allocation{};
        VK_CHECK(vmaCreateBuffer(
            _vma,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &allocation.buffer,
            &allocation.allocation,
            &allocationInfo
        ));

        allocation.ptr = allocationInfo.pMappedData;
        allocation.byte_size = allocationInfo.size;
        return allocation;
    }

    ImageAllocation ResourceAllocator::AllocatePoolImage(VmaPool pool, const VkExtent2D& extent, Image2DAllocationPreset preset)
    {
        const Image2DAllocationPresetVals& presetVals = IMAGE2D_ALLOCATION_PRESETS[preset];

        VkImageCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VkImageType::VK_IMAGE_TYPE_2D,
            .format = presetVals.format,
            .extent = VkExtent3D{extent.width, extent.height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
            .usage = presetVals.usage_flags,
            .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
        };

        ImageAllocation imageAllocation{};

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .usage = VMA_MEMORY_USAGE_AUTO,
            .pool = pool,
        };


        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateImage(
            _vma,
            &createInfo,
            &allocationCreateInfo,
            &imageAllocation.image,
            &imageAllocation.allocation,
            &allocationInfo
        ));

        return imageAllocation;
    }




    ImageAllocation ResourceAllocator::AllocateImage2D(const VkExtent2D& extent, Image2DAllocationPreset preset, bool dedicated)
    {
        const Image2DAllocationPresetVals& presetVals = IMAGE2D_ALLOCATION_PRESETS[preset];

        VkImageCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VkImageType::VK_IMAGE_TYPE_2D,
            .format = presetVals.format,
            .extent = VkExtent3D{extent.width, extent.height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
            .usage = presetVals.usage_flags,
            .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
        };

        ImageAllocation imageAllocation{};

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        if (dedicated)
        {
            allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }
        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateImage(
            _vma,
            &createInfo,
            &allocationCreateInfo,
            &imageAllocation.image,
            &imageAllocation.allocation,
            &allocationInfo
        ));

        return imageAllocation;
    }

    //std::optional<BufferAllocation> ResourceAllocator::TryAllocatePoolBuffer(VmaPool pool, [[maybe_unused]] uint64_t byteSize)
    //{
    //    VmaAllocationCreateInfo allocationCreateInfo
    //    {
    //        .pool = pool
    //    };
    //    VmaAllocationInfo allocationInfo{};
    //    BufferAllocation bufferAllocation{};
    //    auto result = vmaCreateBuffer(
    //        _vma,
    //        nullptr,
    //        &allocationCreateInfo,
    //        &bufferAllocation.buffer,
    //        &bufferAllocation.allocation,
    //        &allocationInfo
    //    );

    //    if (result != VkResult::VK_SUCCESS)
    //    {
    //        return std::nullopt;
    //    }

    //    assert(allocationInfo.pMappedData);
    //    assert(allocationInfo.size == byteSize);

    //    return bufferAllocation;
    //}
    
    void ResourceAllocator::DeallocateBuffer(const BufferAllocation& bufferAllocation)
    {
        vmaDestroyBuffer(_vma, bufferAllocation.buffer, bufferAllocation.allocation);
    }

    void ResourceAllocator::DeallocatePool(const PoolAllocation& poolAllocation)
    {
        vmaDestroyPool(_vma, poolAllocation.pool);
    }

    void ResourceAllocator::DeallocateImage(const ImageAllocation& imageAllocation)
    {
        vmaDestroyImage(_vma, imageAllocation.image, imageAllocation.allocation);
    }

    void ResourceAllocator::InvalidateBuffer(const BufferAllocation& bufferAllocation)
    {
        VK_CHECK(vmaInvalidateAllocation(_vma, bufferAllocation.allocation, 0, bufferAllocation.byte_size));
    }

    void ResourceAllocator::FlushBuffer(const BufferAllocation& bufferAllocation)
    {
        VK_CHECK(vmaFlushAllocation(_vma, bufferAllocation.allocation, 0, bufferAllocation.byte_size));
    }

    VkImageView CreateImage2DView(const Device& device, VkImage image, Image2DAllocationPreset preset)
    {
        const Image2DAllocationPresetVals& presetVals = IMAGE2D_ALLOCATION_PRESETS[preset];
       
        VkImageSubresourceRange subResourceRange
        {
            .aspectMask = presetVals.aspect_flags,
            .baseMipLevel = 0,
            .levelCount = presetVals.mip_levels, 
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        VkImageViewCreateInfo imageViewCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = VkImageViewCreateFlags{},
            .image = image,
            .viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
            .format = presetVals.format,
            .components = VkComponentMapping{},
            .subresourceRange = subResourceRange
        };

        return device.CreateImageView(imageViewCreateInfo);
    }


 



}