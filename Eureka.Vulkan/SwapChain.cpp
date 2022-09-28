#include "SwapChain.hpp"
#include <debugger_trace.hpp>
#include <assert.hpp>


namespace eureka::vulkan
{


    VkExtent2D CalcMaxExtent(uint32_t width, uint32_t height, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width == UINT32_MAX)
        {
            return VkExtent2D
            {
                .width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                .height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }
        else
        {
            assert(capabilities.currentExtent.width == width);
            assert(capabilities.currentExtent.height == height);
            return capabilities.currentExtent;
        }
    }

	//SwapChainSupportDetails QuerySwapchainSupport(
	//	const vkr::PhysicalDevice* device, 
	//	VkSurfaceKHR surface
 //   )
	//{
 //       /*	SwapChainSupportDetails support;

 //           support.capabilities = device->getSurfaceCapabilitiesKHR(surface);
 //           support.formats = device->getSurfaceFormatsKHR(surface);
 //           support.present_modes = device->getSurfacePresentModesKHR(surface);*/

 //  //     DEBUGGER_TRACE(
 //  //         R"(
	//		//SwapChain can support the following capabilites
	//		//minimum image count = {}
	//		//maximum image count = {}
	//		//current extent = ({}, {})
	//		//min extent = ({}, {})
	//		//max extent = ({}, {})
	//		//maximum image array layers = {}
	//		//supported transforms = {}
	//		//current transform = {}
	//		//supported composite alpha = {}
	//		//supported usage flags = {}
	//		//supported formats = {}
	//		//supported color spaces = {}
	//		//supported present modes = {}
	//		//)",
 //  //         support.capabilities.minImageCount,
 //  //         support.capabilities.maxImageCount,
 //  //         support.capabilities.currentExtent.width, support.capabilities.currentExtent.height,
 //  //         support.capabilities.minImageExtent.width, support.capabilities.minImageExtent.height,
 //  //         support.capabilities.maxImageExtent.width, support.capabilities.maxImageExtent.height,
 //  //         support.capabilities.maxImageArrayLayers,
 //  //         support.capabilities.supportedTransforms,
 //  //         support.capabilities.currentTransform,
 //  //         support.capabilities.supportedCompositeAlpha,
 //  //         support.capabilities.supportedUsageFlags,
 //  //         support.formats | std::views::transform([](const auto& v) {return v.format; }),
 //  //         support.formats | std::views::transform([](const auto& v) {return v.colorSpace; }),
 //  //         support.present_modes
 //  //     );
	//	return support;
	//}

    SwapChain::SwapChain(
        std::shared_ptr<Window> window,
        std::shared_ptr<Device> device,
        Queue presentQueue,
        Queue graphicsQueue
    )
        :
        _window(std::move(window)),
        _device(std::move(device)),
        _presentationQueue(presentQueue),
        _graphicsQueue(graphicsQueue)
    {
        auto surface = _window->GetSurface();
		auto [capabilities, formats, presentModes] = _device->GetSwapChainSupport(surface);
    
        auto fitr = std::ranges::find_if(
            formats,
            [](const VkSurfaceFormatKHR& sf) 
            { return sf.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM && sf.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; }
        );


        auto pitr = std::ranges::find_if(
            presentModes,
            [](const VkPresentModeKHR& pm) { return pm == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR; }
        );


        _capabilities = capabilities;
		_surfaceFormat = (fitr != formats.end()) ? *fitr : formats.at(0);
        _selectedPresentMode = (pitr != presentModes.end()) ? VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR : VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR; // currently we don't use mailbox

        auto deviceName = _device->GetPrettyName();

        if (deviceName.contains("Intel") && deviceName.contains("UHD"))
        {
            DEBUGGER_TRACE("GPU Identified as Intel UHD, setting presentation mode to immediate - possible driver bug");
            _selectedPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

		CreateSwapChain();

        //
        // sync objects
        //

        //vk::FenceCreateInfo fenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
        for (auto i = 0u; i < _surfaceImages.size(); ++i)
        {
            _imageReadySemapores.emplace_back(_device);

        }
        _windowResizeConnection = _window->ConnectResizeSlot([this](uint32_t w, uint32_t h)
            {
                Resize(w, h);
            });
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        if (width > 0 && height > 0)
        {
            _capabilities = _capabilities = _device->GetSwapchainCapabilities(_window->GetSurface()); 
            _currentSemaphore = 0;
            CreateSwapChain();
            _resizeSignal(width, height);
        }


    }

    SwapChainImageReference SwapChain::AcquireNextAvailableImageAsync()
    {
        // https://stackoverflow.com/questions/65054157/do-i-need-dedicated-fences-semaphores-per-swap-chain-image-per-frame-or-per-com

        if (_outOfData)
        {
            return SwapChainImageReference
            {
                .valid = false
            };
        }

        auto semaphoreHandle = _imageReadySemapores[_currentSemaphore].Get();

        auto [result, imageIndex] = _device->AcquireNextSwapChainImage(_swapchain, UINT64_MAX, semaphoreHandle, nullptr);

        //auto [result, index] = _swapchain.acquireNextImage(UINT64_MAX, semaphoreHandle);
        if (result == VkResult::VK_SUCCESS)
        {
            _currentSemaphore = (_currentSemaphore + 1) % _imageReadySemapores.size();
            _lastAquiredImage = imageIndex;
            return SwapChainImageReference
            {
                .valid = true,
                .image_index = imageIndex,
                .image_ready = semaphoreHandle
            };

        }
        else if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR || result == VkResult::VK_SUBOPTIMAL_KHR)
        {
            _outOfData = true;
            CreateSwapChain();
        }

        return SwapChainImageReference
        {
            .valid = false,
            .image_index = imageIndex,
            .image_ready = semaphoreHandle
        };

 


    }

    VkResult SwapChain::PresentLastAcquiredImageAsync(const BinarySemaphoreHandle& renderingDoneSemaphore)
    {
        VkSemaphore sem = renderingDoneSemaphore.Get();
        VkPresentInfoKHR presentInfo
        {
             .sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
             .waitSemaphoreCount = 1,
             .pWaitSemaphores = &sem,
             .swapchainCount = 1,
             .pSwapchains = &_swapchain,
             .pImageIndices = &_lastAquiredImage
        };
      
   
        
        auto result = _presentationQueue.Present(presentInfo);

        if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
        {
            //DEBUGGER_TRACE("out of date");
            _outOfData = true;
            CreateSwapChain();
        }

        return result;
    }

    VkRect2D SwapChain::RenderArea() const
    {
        return VkRect2D
        {
            {0u, 0u},
            {_window->GetWidth(), _window->GetHeight()}
        };
    }

    std::vector<std::shared_ptr<Image>> SwapChain::Images() const
    {
        return _surfaceImages;
    }

    void SwapChain::CreateSwapChain()
    {
        auto width = _window->GetWidth();
        auto height = _window->GetHeight();
        auto surface = _window->GetSurface();
        if (width > 0 && height > 0)
        {
            VkExtent2D selectedExtent = CalcMaxExtent(width, height, _capabilities);

            uint32_t minImageCount = std::max(2u, _capabilities.minImageCount);
            VkSwapchainCreateInfoKHR createInfo
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .flags = VkSwapchainCreateFlagsKHR{},
                .surface = surface,
                .minImageCount = minImageCount,
                .imageFormat = _surfaceFormat.format,
                .imageColorSpace = _surfaceFormat.colorSpace,
                .imageExtent = selectedExtent,
                .imageArrayLayers = 1,
                .imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .preTransform = _capabilities.currentTransform,
                .compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = _selectedPresentMode,
                .clipped = VK_TRUE
            };

            std::array<uint32_t, 2> queueFamiliyIndices{ _graphicsQueue.Family(), _presentationQueue.Family() };
            if (_graphicsQueue.Family() != _presentationQueue.Family())
            {
                createInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamiliyIndices.data();
            }
            else
            {
                createInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
            }
            _swapchain = nullptr; // first release then create
            _swapchain = _device->CreateSwapchain(createInfo);

            auto images = _device->GetSwapchainImages(_swapchain);

            _surfaceImages.resize(images.size(), nullptr);

            for (auto i = 0u; i < images.size(); ++i)
            {
                VkImageSubresourceRange subResourceRange
                {
                    .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1, // no mip map for now
                    .baseArrayLayer = 0,
                    .layerCount = 1
                };

                VkImageViewCreateInfo imageViewCreateInfo
                {
                    .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .flags = VkImageViewCreateFlags{},
                    .image = images[i],
                    .viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
                    .format = _surfaceFormat.format,
                    .components = VkComponentMapping{},
                    .subresourceRange = subResourceRange
                };

                auto view = _device->CreateImageView(imageViewCreateInfo);
                _surfaceImages[i] = std::make_shared<Image>(_device,images[i], std::move(view));
            }

            _outOfData = false;
        }

        
    }

    uint32_t SwapChain::ImageCount() const
    {
        return static_cast<uint32_t>(_surfaceImages.size());
    }

    SwapChain::~SwapChain()
    {
        _device->DestroySwapChain(_swapchain);
    }

 

}