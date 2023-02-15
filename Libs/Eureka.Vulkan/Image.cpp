#include "Image.hpp"
#include "Result.hpp"
#include <move.hpp>
namespace eureka::vulkan
{

    //////////////////////////////////////////////////////////////////////////
    //
    //                         Image
    //
    //////////////////////////////////////////////////////////////////////////

    Image::~Image() noexcept
    {
        if(nullptr != _view)
        {
            _device->DestroyImageView(_view);
        }
    }

    Image::Image(Image&& that) noexcept :
        _device(std::move(that._device)),
        _allocation(that._allocation),
        _view(steal(that._view))
    {
        that._allocation = {};
    }

    Image& Image::operator=(Image&& rhs) noexcept
    {
        _device = std::move(rhs._device);
        _allocation = rhs._allocation;
        _view = steal(rhs._view);
        rhs._allocation = {};
        return *this;
    }

    Image::Image(std::shared_ptr<Device> device, VkImage image, VkImageView view) :
        _device(std::move(device)),
        _view(view)
    {
        _allocation.image = image;
    }

    Image::Image(std::shared_ptr<Device> device) :
        _device(std::move(device))
    {}

    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedImage
    //
    //////////////////////////////////////////////////////////////////////////

    AllocatedImage::~AllocatedImage() noexcept
    {
        Deallocate();
    }

    AllocatedImage::AllocatedImage(std::shared_ptr<Device> device, std::shared_ptr<ResourceAllocator> allocator) :
        Image(std::move(device)),
        _allocator(std::move(allocator))
    {}

    void AllocatedImage::Allocate(const Image2DProperties& props)
    {
        Deallocate();

        _allocation = _allocator->AllocateImage2D(props.extent, props.preset);
        _view = CreateImage2DView(*_device, _allocation.image, props.preset);
    }

    void AllocatedImage::Deallocate()
    {
        if(nullptr != _allocation.image)
        {
            _allocator->DeallocateImage(_allocation);
        }
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //                        PoolAllocatedImage
    //
    //////////////////////////////////////////////////////////////////////////

    PoolAllocatedImage::~PoolAllocatedImage() noexcept
    {
        Deallocate();
    }

    void PoolAllocatedImage::Allocate(const Image2DProperties& props)
    {
        Deallocate();

        _allocation = _allocator->AllocateImage(props.extent);
        _view = CreateImage2DView(*_device, _allocation.image, props.preset);
    }

    void PoolAllocatedImage::Deallocate()
    {
        if (nullptr != _allocation.image)
        {
            _allocator->DeallocateImage(_allocation);
        }
    }
    PoolAllocatedImage::PoolAllocatedImage(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator) :
        Image(std::move(device)),
        _allocator(std::move(allocator))
    {
    }



    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedImage2D
    //
    //////////////////////////////////////////////////////////////////////////

    AllocatedImage2D::AllocatedImage2D(std::shared_ptr<Device> device,
                     std::shared_ptr<ResourceAllocator> allocator,
                     const Image2DProperties& props) :
        AllocatedImage(std::move(device), std::move(allocator))
    {
        Allocate(props);
    }

    AllocatedImage2D::AllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ResourceAllocator> allocator) :
        AllocatedImage(std::move(device), std::move(allocator))
    {}

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PoolAllocatedImage2D
    //
    //////////////////////////////////////////////////////////////////////////

    PoolAllocatedImage2D::PoolAllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator, const Image2DProperties& props)
        : PoolAllocatedImage(std::move(device), std::move(allocator))
    {
        Allocate(props);
    }

    PoolAllocatedImage2D::PoolAllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator)
        : PoolAllocatedImage(std::move(device), std::move(allocator))
    {
    
    }



    //////////////////////////////////////////////////////////////////////////
    //
    //                     SampledImage2D (linear sampling)
    //
    //////////////////////////////////////////////////////////////////////////

    //SampledImage2D::SampledImage2D(const DeviceContext& deviceContext, const Image2DProperties& props)
    //    : Image2D(deviceContext, props)
    //{
    //    vk::SamplerCreateInfo samplerCreateInfo
    //    {
    //        .magFilter = vk::Filter::eLinear,
    //        .minFilter = vk::Filter::eLinear,
    //        .mipmapMode = vk::SamplerMipmapMode::eLinear,
    //        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
    //        .addressModeV = vk::SamplerAddressMode::eClampToEdge,
    //        .addressModeW = vk::SamplerAddressMode::eClampToEdge,
    //        .mipLodBias = 0.0f,
    //        .compareOp = vk::CompareOp::eNever,
    //        .minLod = 0.0f
    //    };

    //    _sampler = deviceContext.LogicalDevice()->createSampler(samplerCreateInfo);
    //}

    //SampledImage2D::SampledImage2D(SampledImage2D&& that)
    //    : Image2D(std::move(that)), _sampler(std::move(that._sampler))
    //{

    //}

    //SampledImage2D::~SampledImage2D()
    //{

    //}

    //SampledImage2D& SampledImage2D::SampledImage2D::operator=(SampledImage2D&& rhs)
    //{
    //    Image2D::operator=(std::move(rhs));
    //    _sampler = std::move(rhs._sampler);
    //    return *this;
    //}

    Sampler::Sampler(std::shared_ptr<Device> device, const VkSamplerCreateInfo& createInfo)
        : 
        _device(std::move(device)),
        _sampler(_device->CreateSampler(createInfo))
    {
    }

    Sampler::Sampler(Sampler&& that) noexcept
        : 
        _device(std::move(that._device)),
        _sampler(steal(that._sampler))
    {
    }

    Sampler& Sampler::operator=(Sampler&& rhs) noexcept
    {
        _device = std::move(rhs._device);
        _sampler = steal(rhs._sampler);
        return *this;
    }

    Sampler::~Sampler()
    {
        if (nullptr != _sampler)
        {
            _device->DestroySampler(_sampler);
        }
    }

    Sampler CreateSampler(std::shared_ptr<Device> device, SamplerCreationPreset preset)
    {
        VkSamplerCreateInfo samplerCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
        };
        
        if (preset == SamplerCreationPreset::eLinearClampToEdge)
        {
            samplerCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
            samplerCreateInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCreateInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.mipLodBias = 0.0f;
            samplerCreateInfo.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
            samplerCreateInfo.minLod = 0.0f;
        }
        else
        {
            throw std::logic_error("bad");
        }

        return Sampler(std::move(device), samplerCreateInfo);
    }





} // namespace eureka::vulkan