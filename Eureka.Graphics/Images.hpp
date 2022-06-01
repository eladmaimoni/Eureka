
#include "DeviceContext.hpp"
#include "vk_error_handling.hpp"

namespace eureka
{



    struct Image2DProperties 
    {
        uint32_t             width;
        uint32_t             height;
        vk::Format           format;
        vk::ImageUsageFlags  usage_flags;
        vk::ImageAspectFlags aspect_flags;
    };

    class Image
    {
    public:
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image() = default;
        Image(const DeviceContext& deviceContext);
        Image(Image&& that); 
        Image& operator=(Image&& rhs);
    protected:
        vk::Image         _image{nullptr};
        VmaAllocator      _allocator{ nullptr };
        VmaAllocation     _allocation{ nullptr };
        VmaAllocationInfo _allocationInfo{  };
        vkr::ImageView    _view{ nullptr };

    };


    vkr::ImageView CreateImage2DView(const DeviceContext& deviceContext, vk::Image image, const Image2DProperties& props);


    class Image2D : Image
    {
    public:
        Image2D() = default;      
        Image2D(const DeviceContext& deviceContext, const Image2DProperties& props);
        Image2D(Image2D&& that) : Image(std::move(that)) {}
        Image2D& operator=(Image2D&& rhs);
        ~Image2D();
    };
}