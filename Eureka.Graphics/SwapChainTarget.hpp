#include <DeviceContext.hpp>
#include <Images.hpp>

namespace eureka
{
    struct SwapChainTargetDesc
    {
        vkr::SurfaceKHR                      surface{nullptr};
        uint32_t                                  width;
        uint32_t                                  height;
        uint32_t                                  present_queue_family;
        uint32_t                                  graphics_queue_family;
    };

    class SwapChainTarget
    {
    public:
        SwapChainTarget(
            DeviceContext& deviceContext, 
            SwapChainTargetDesc desc
        );

        void Resize(uint32_t width, uint32_t height);



    private:

        SwapChainTargetDesc              _desc;
        DeviceContext&                   _deviceContext;

        vk::Format                       _swapchainFormat;
        vk::Extent2D                     _swapchainExtent;
        vkr::SwapchainKHR                _swapchain{ nullptr };
        std::vector<VkImage>             _images;
        std::vector<vkr::ImageView>      _imageViews;
        vk::SurfaceFormatKHR             _surfaceFormat;
        vk::PresentModeKHR               _selectedPresentMode;
        Image2D                          _depthImage;


        
        void CreateSwapChain();
        void CreateDepthBuffer();
        void CreateFrameBuffer();


        vk::SurfaceCapabilitiesKHR _capabilities;
    public:
        uint32_t ImageCount() const;
    };
}