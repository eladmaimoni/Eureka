#pragma once
#include "Device.hpp"
#include "Image.hpp"
#include "Synchronization.hpp"
#include <sigslot/signal.hpp>
#include "../Eureka.Windowing/Window.hpp"

namespace eureka::vulkan
{

    struct SwapChainImageReference
    {
        bool          valid = false;
        uint32_t      image_index{ 0u };
        VkSemaphore   image_ready{ nullptr };
    };

    class SwapChain
    {
    public:
        SwapChain(
            std::shared_ptr<Window> window,
            std::shared_ptr<Device> device,
            Queue presentQueue,
            Queue graphicsQueue
        );
        ~SwapChain();

        SwapChainImageReference AcquireNextAvailableImageAsync();
        VkResult PresentLastAcquiredImageAsync(const BinarySemaphoreHandle& renderingDoneSemaphore);

        VkRect2D RenderArea() const;
        VkFormat ImageFormat() const { return _surfaceFormat.format; }
        std::vector<std::shared_ptr<Image>> Images() const;
        uint32_t ImageCount() const;

        template <typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            auto width = _window->GetWidth();
            auto height = _window->GetHeight();
            if (width > 0 && height > 0)
            {
                slot(width, height);
            }
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }
    private:
        std::shared_ptr<Window>              _window;
        std::shared_ptr<Device>              _device;
        sigslot::scoped_connection           _windowResizeConnection;
        Queue                                _graphicsQueue;
        Queue                                _presentationQueue;
        VkExtent2D                           _swapchainExtent;
        VkSwapchainKHR                       _swapchain{ nullptr };
        VkSurfaceFormatKHR                   _surfaceFormat;
        VkPresentModeKHR                     _selectedPresentMode;
        std::vector<std::shared_ptr<Image>>  _surfaceImages;
        std::vector<BinarySemaphore>         _imageReadySemapores;  // TODO probably no need for multiple semaphores?
        VkSurfaceCapabilitiesKHR             _capabilities;
        uint32_t                             _currentSemaphore{ 0 };
        uint32_t                             _lastAquiredImage{ 0 };
        bool                                 _outOfData{ true };
    //    //uint32_t                           _currentSemaphore = 0;

        sigslot::signal<uint32_t, uint32_t>  _resizeSignal;

        void Resize(uint32_t width, uint32_t height);
        void CreateSwapChain();
    };



}