#pragma once
#include "ResourceAllocator.hpp"
#include "ImageMemoryPool.hpp"


namespace eureka::vulkan
{

    //////////////////////////////////////////////////////////////////////////
    //
    //                        Image (non owning)
    //
    //////////////////////////////////////////////////////////////////////////

    class Image
    {
    public:
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
    public:
        virtual ~Image() noexcept;
        Image() = default;
        Image(Image&& that) noexcept;
        Image& operator=(Image&& rhs) noexcept;
        Image(std::shared_ptr<Device> device, VkImage image, VkImageView view);
        Image(std::shared_ptr<Device> device);
        VkImage Get() const { return _allocation.image; }
        VkImageView GetView() const { return _view; }
    protected:
        std::shared_ptr<Device> _device;
        ImageAllocation         _allocation{}; // note: non owning
        VkImageView             _view{ nullptr }; // note: owning
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedImage
    //
    //////////////////////////////////////////////////////////////////////////

    struct Image2DProperties
    {
        VkExtent2D extent{};
        Image2DAllocationPreset preset{};
    };

    class AllocatedImage : public Image
    {
    public:
        virtual ~AllocatedImage() noexcept;

        void Allocate(const Image2DProperties& props);
        void Deallocate();

        AllocatedImage(AllocatedImage&& that) noexcept = default;
        AllocatedImage& operator=(AllocatedImage&& rhs) noexcept = default;
        AllocatedImage(std::shared_ptr<Device> device, std::shared_ptr<ResourceAllocator> allocator);
    protected:
        std::shared_ptr<ResourceAllocator> _allocator;
    };

    class PoolAllocatedImage : public Image
    {
    public:
        virtual ~PoolAllocatedImage() noexcept;

        void Allocate(const Image2DProperties& props);
        void Deallocate();

        PoolAllocatedImage(PoolAllocatedImage&& that) noexcept = default;
        PoolAllocatedImage& operator=(PoolAllocatedImage&& rhs) noexcept = default;
        PoolAllocatedImage(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator);
    protected:
        std::shared_ptr<ImageMemoryPool> _allocator;
    };

    


    //////////////////////////////////////////////////////////////////////////
    //
    //                        Image2D
    //
    //////////////////////////////////////////////////////////////////////////


    class AllocatedImage2D : public AllocatedImage
    {
    public:
        AllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ResourceAllocator> allocator);
        AllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ResourceAllocator> allocator, const Image2DProperties& props);
        AllocatedImage2D(AllocatedImage2D&& that) noexcept = default;
        AllocatedImage2D& operator=(AllocatedImage2D&& rhs) noexcept = default;
    };


    class PoolAllocatedImage2D : public PoolAllocatedImage
    {
    public:
        PoolAllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator);
        PoolAllocatedImage2D(std::shared_ptr<Device> device, std::shared_ptr<ImageMemoryPool> allocator, const Image2DProperties& props);
        PoolAllocatedImage2D(PoolAllocatedImage2D&& that) noexcept = default;
        PoolAllocatedImage2D& operator=(PoolAllocatedImage2D&& rhs) noexcept = default;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                     SampledImage2D (linear sampling)
    //
    //////////////////////////////////////////////////////////////////////////


    enum SamplerCreationPreset
    {
        eLinearClampToEdge
    };

    class Sampler 
    {
        std::shared_ptr<Device> _device;
        VkSampler _sampler{ nullptr };
    public:
        Sampler() = default;
        Sampler(std::shared_ptr<Device> device, const VkSamplerCreateInfo& createInfo);
        Sampler(Sampler&& that) noexcept;
        Sampler& operator=(Sampler&& rhs) noexcept;
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        ~Sampler();
        VkSampler Get() const
        {
            return _sampler;
        }
    };

    Sampler CreateSampler(std::shared_ptr<Device> device, SamplerCreationPreset preset);

    //class MipmapImage2D : public AllocatedImage
    //{
    //    // TODO: mipmapped image
    //    // https://vulkan-tutorial.com/Generating_Mipmaps
    //};

  
}