#include "Images.hpp"

namespace eureka
{
    //void CreateImage2D(const DeviceContext& deviceContext, const Image2DProperties& props)
    //{
    //    //Image2D image2d;

    //    //
    //    // create image object
    //    //
    //    vk::ImageCreateInfo createInfo
    //    {
    //        .imageType = vk::ImageType::e2D,
    //        .format = props.format,
    //        .extent = vk::Extent3D{props.width, props.height, 1},
    //        .mipLevels = 1,
    //        .arrayLayers = 1,
    //        .samples = vk::SampleCountFlagBits::e1,
    //        .tiling = vk::ImageTiling::eOptimal,
    //        .usage = props.usage_flags,
    //        .sharingMode = vk::SharingMode::eExclusive,
    //        .initialLayout = vk::ImageLayout::eUndefined,
    //    };
 
    //    auto image = deviceContext.LogicalDevice()->createImage(createInfo);
    //    auto imageMemoryRequirements = deviceContext.LogicalDevice()->getImageMemoryRequirements2(
    //        vk::ImageMemoryRequirementsInfo2{
    //            .image = *image
    //        }    
    //        ).memoryRequirements;

    //    auto deviceMemoryProps = deviceContext.PhysicalDevice()->getMemoryProperties();
    //    
    //    std::bitset<32> supportedMemoryTypeBitsFilter = imageMemoryRequirements.memoryTypeBits;

    //    //
    //    // find the memory type that supports this resource type (image) and desired 'properties' (device memory)
    //    //

    //    auto compatibleMemoryTypeIndex = 0xFFFF'FFFF;

    //    for (auto i = 0u; i < deviceMemoryProps.memoryTypeCount; ++i)
    //    {
    //        if (supportedMemoryTypeBitsFilter[i] &&
    //            (deviceMemoryProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal
    //            )
    //        {
    //            compatibleMemoryTypeIndex = i;
    //            break;
    //        }
    //    }

    //    if (compatibleMemoryTypeIndex == 0xFFFF'FFFF)
    //    {
    //        
    //        throw vk::SystemError(vk::Result::eErrorUnknown);
    //    }

    //    //
    //    // allocate memory for the image object
    //    //

    //    vk::MemoryAllocateInfo memoryAllocateInfo
    //    {
    //       .allocationSize = imageMemoryRequirements.size,
    //       .memoryTypeIndex = compatibleMemoryTypeIndex
    //    };

    //    auto memory = deviceContext.LogicalDevice()->allocateMemory(memoryAllocateInfo);
    //    
    //    deviceContext.LogicalDevice()->bindImageMemory2(
    //        vk::BindImageMemoryInfo{
    //            .image = *image,
    //            .memory = *memory,
    //            .memoryOffset = 0
    //        }
    //    );
    //    auto view = CreateImage2DView(deviceContext, *image, props);


    //    //return image2d;
    //}

    vkr::ImageView CreateImage2DView(const DeviceContext& deviceContext, vk::Image image, const Image2DProperties& props)
    {
        vk::ImageSubresourceRange subResourceRange
        {
            .aspectMask = props.aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1, // no mip map for now
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        vk::ImageViewCreateInfo imageViewCreateInfo
        {
            .flags = vk::ImageViewCreateFlags(),
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = props.format,
            .components = vk::ComponentMapping(),
            .subresourceRange = subResourceRange
        };

        return deviceContext.LogicalDevice()->createImageView(imageViewCreateInfo);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                         Image
    //
    //////////////////////////////////////////////////////////////////////////

    Image::Image(const DeviceContext& deviceContext) : _allocator(deviceContext.Allocator())
    {

    }

    Image::Image(Image&& that) :
        _image(that._image),
        _allocation(that._allocation),
        _view(std::move(that._view)),
        _allocationInfo(that._allocationInfo),
        _allocator(that._allocator)
    {
        that._image = nullptr;
        that._allocation = nullptr;
        that._allocator = nullptr;
    }

    Image& Image::operator=(Image&& rhs)
    {
        _image = rhs._image;
        _allocation = rhs._allocation;
        _view = std::move(rhs._view);
        _allocationInfo = rhs._allocationInfo;
        _allocator = rhs._allocator;

        rhs._image = nullptr;
        rhs._allocation = nullptr;

        return *this;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                        Image2D
    //
    //////////////////////////////////////////////////////////////////////////

    Image2D::Image2D(const DeviceContext& deviceContext, const Image2DProperties& props) : Image(deviceContext)
    {
        vk::ImageCreateInfo createInfo
        {
            .imageType = vk::ImageType::e2D,
            .format = props.format,
            .extent = vk::Extent3D{props.width, props.height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = props.usage_flags,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined,
        };

        VmaAllocationCreateInfo allocationCreateInfo
        {
            .usage = VMA_MEMORY_USAGE_GPU_ONLY
        };

        VmaAllocationInfo allocationInfo{};

        VK_CHECK(vmaCreateImage(
            _allocator,
            &reinterpret_cast<VkImageCreateInfo&>(createInfo),
            &allocationCreateInfo,
            &reinterpret_cast<VkImage&>(_image),
            &_allocation,
            &allocationInfo
        ));

        _view = CreateImage2DView(deviceContext, _image, props);
    }

    Image2D::~Image2D()
    {
        if (_image)
        {
            vmaDestroyImage(_allocator, _image, _allocation);
        }
    }

    Image2D& Image2D::operator=(Image2D&& rhs)
    {
        if (_image)
        {
            vmaDestroyImage(_allocator, _image, _allocation);
        }

        Image::operator=(std::move(rhs));

        return *this;
    }



}