#pragma once
#include "DeviceContext.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{


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
        vk::ImageView View() const { return *_view; }
    protected:
        vk::Image         _image{nullptr}; // note: non owning
        vkr::ImageView    _view{ nullptr }; // note: owning
    };


    class AllocatedImage : public Image
    {
    public:
        AllocatedImage(AllocatedImage&& that);
        AllocatedImage& operator=(AllocatedImage&& rhs);        
        AllocatedImage(const DeviceContext& deviceContext);
    protected:
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
        VmaAllocationInfo _allocationInfo{  };
    };

    struct Image2DProperties
    {
        uint32_t             width;
        uint32_t             height;
        vk::Format           format;
        vk::ImageUsageFlags  usage_flags;
        vk::ImageAspectFlags aspect_flags;
    };

    vkr::ImageView CreateImage2DView(const DeviceContext& deviceContext, vk::Image image, const Image2DProperties& props);
   
    class Image2D : public AllocatedImage
    {
    public:
        Image2D() = default;      
        Image2D(const DeviceContext& deviceContext, const Image2DProperties& props);
        Image2D(Image2D&& that) : AllocatedImage(std::move(that)) {}
        Image2D& operator=(Image2D&& rhs);
        ~Image2D();
    };


    Image2D CreateDepthImage(const DeviceContext& deviceContext, uint32_t width, uint32_t height);
}