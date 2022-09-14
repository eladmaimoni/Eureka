#pragma once
#include "DeviceContext.hpp"


namespace eureka
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
        virtual ~Image();
        Image() = default;
        Image(Image&& that); 
        Image& operator=(Image&& rhs);
        Image(vk::Image image, vkr::ImageView view);
        vk::Image Get() const { return _image; };
        vk::ImageView GetView() const { return *_view; }
    protected:
        vk::Image         _image{nullptr}; // note: non owning
        vkr::ImageView    _view{ nullptr }; // note: owning
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        AllocatedImage
    //
    //////////////////////////////////////////////////////////////////////////

    class AllocatedImage : public Image
    {
    public:
        virtual ~AllocatedImage();
        AllocatedImage() = default;
        AllocatedImage(AllocatedImage&& that);
        AllocatedImage& operator=(AllocatedImage&& rhs);        
        AllocatedImage(const DeviceContext& deviceContext);
    protected:
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
    };



    struct Image2DProperties
    {
        uint32_t             width;
        uint32_t             height;
        vk::Format           format;
        vk::ImageUsageFlags  usage_flags;
        vk::ImageAspectFlags aspect_flags;
        bool                 use_dedicated_memory_allocation{false};
    };

    vkr::ImageView CreateImage2DView(const DeviceContext& deviceContext, vk::Image image, const Image2DProperties& props);
   

    //////////////////////////////////////////////////////////////////////////
    //
    //                        Image2D
    //
    //////////////////////////////////////////////////////////////////////////
    class Image2D : public AllocatedImage
    {
    public:
        Image2D() = default;
        Image2D(const DeviceContext& deviceContext, const Image2DProperties& props);
        Image2D(Image2D&& that) noexcept : AllocatedImage(std::move(that)) {}
        Image2D& operator=(Image2D&& rhs);
        ~Image2D();
    };


    Image2D CreateDepthImage(const DeviceContext& deviceContext, vk::Format format, uint32_t width, uint32_t height);

    //////////////////////////////////////////////////////////////////////////
    //
    //                     SampledImage2D (linear sampling)
    //
    //////////////////////////////////////////////////////////////////////////

    class SampledImage2D : public Image2D
    {
        //
        // TODO not sure this is the right thing to do
        // we can reuse the sampler over many images?
        //
        vkr::Sampler _sampler{ nullptr };
    public:
        SampledImage2D() = default;
        SampledImage2D(const DeviceContext& deviceContext, const Image2DProperties& props);
        SampledImage2D(SampledImage2D&& that);
        SampledImage2D& operator=(SampledImage2D&& rhs);
        ~SampledImage2D();
        vk::Sampler GetSampler() const
        {
            return *_sampler;
        }
    };

    class MipmapImage2D : public AllocatedImage
    {
        // TODO: mipmapped image
        // https://vulkan-tutorial.com/Generating_Mipmaps
    };

  
}