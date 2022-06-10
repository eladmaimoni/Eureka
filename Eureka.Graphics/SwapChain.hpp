#include "DeviceContext.hpp"
#include "Image.hpp"


namespace eureka
{
    //std::array<vk::ClearValue, 2> _clearValues;

    struct SwapChainTargetConfig
    {
        vkr::SurfaceKHR                      surface{nullptr};
        
        uint32_t                             width;
        uint32_t                             height;
        uint32_t                             present_queue_family;
        uint32_t                             graphics_queue_family;

    };

    struct SwapChainImageReference
    {
        uint32_t      image_index{ 0u };
        vk::Semaphore image_ready{nullptr};
    };

    class SwapChain
    {
    public:
        SwapChain(
            DeviceContext& deviceContext,
            SwapChainTargetConfig desc
        );

        void Resize(uint32_t width, uint32_t height);
        SwapChainImageReference AcquireNextAvailableImageAsync();
        vk::Result PresentLastAcquiredImageAsync(vk::Semaphore renderingDoneSemaphore);

        vk::Rect2D RenderArea() const;
        vk::Format ImageFormat() const { return _surfaceFormat.format; }
        std::vector<std::shared_ptr<Image>> Images() const;
        uint32_t ImageCount() const;


    private:
        DeviceContext& _deviceContext;
        SwapChainTargetConfig                _desc;
        vk::Queue                            _presentationQueue;
        vk::Extent2D                         _swapchainExtent;
        vkr::SwapchainKHR                    _swapchain{ nullptr };
        vk::SurfaceFormatKHR                 _surfaceFormat;
        vk::PresentModeKHR                   _selectedPresentMode;
        std::vector<std::shared_ptr<Image>>  _surfaceImages;
        std::vector<vkr::Semaphore>          _imageReadySemapores;  // TODO probably no need for multiple semaphores?
        vk::SurfaceCapabilitiesKHR           _capabilities;
        uint32_t                             _currentSemaphore{ 0 };
        uint32_t                             _lastAquiredImage{ 0 };
        //uint32_t                             _currentSemaphore = 0;
        void CreateSwapChain();
    };


}