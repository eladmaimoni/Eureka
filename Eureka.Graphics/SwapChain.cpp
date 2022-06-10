#include "SwapChain.hpp"
#include "VkHelpers.hpp"
#include <debugger_trace.hpp>



namespace eureka
{
    struct SwapChainSupportDetails 
    {
        vk::SurfaceCapabilitiesKHR        capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR>   present_modes;
    };

	vk::Extent2D CalcMaxExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& capabilities)
	{   
        if (capabilities.currentExtent.width == UINT32_MAX)
        {
			return vk::Extent2D
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

	SwapChainSupportDetails QuerySwapchainSupport(
		const vkr::PhysicalDevice* device, 
		vk::SurfaceKHR surface
    ) 
	{
		SwapChainSupportDetails support;

		support.capabilities = device->getSurfaceCapabilitiesKHR(surface);
		support.formats = device->getSurfaceFormatsKHR(surface);
		support.present_modes = device->getSurfacePresentModesKHR(surface);

        DEBUGGER_TRACE(
            R"(
			SwapChain can support the following capabilites
			minimum image count = {}
			maximum image count = {}
			current extent = ({}, {})
			min extent = ({}, {})
			max extent = ({}, {})
			maximum image array layers = {}
			supported transforms = {}
			current transform = {}
			supported composite alpha = {}
			supported usage flags = {}
			supported formats = {}
			supported color spaces = {}
			supported present modes = {}
			)",
            support.capabilities.minImageCount,
            support.capabilities.maxImageCount,
            support.capabilities.currentExtent.width, support.capabilities.currentExtent.height,
            support.capabilities.minImageExtent.width, support.capabilities.minImageExtent.height,
            support.capabilities.maxImageExtent.width, support.capabilities.maxImageExtent.height,
            support.capabilities.maxImageArrayLayers,
            support.capabilities.supportedTransforms,
            support.capabilities.currentTransform,
            support.capabilities.supportedCompositeAlpha,
            support.capabilities.supportedUsageFlags,
            support.formats | std::views::transform([](const auto& v) {return v.format; }),
            support.formats | std::views::transform([](const auto& v) {return v.colorSpace; }),
            support.present_modes
        );
		return support;
	}

    SwapChain::SwapChain(
        DeviceContext& deviceContext,
        SwapChainTargetConfig desc
    )
		: 
		_deviceContext(deviceContext),
        _presentationQueue(deviceContext.PresentQueue()),
		_desc(std::move(desc))
    {
		auto [capabilities, formats, presentModes] = QuerySwapchainSupport(_deviceContext.PhysicalDevice().get(), *_desc.surface);

		auto fitr = std::ranges::find_if(
            formats,
			[](const vk::SurfaceFormatKHR& sf) { return sf.format == vk::Format::eB8G8R8A8Unorm && sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; }
		);


        auto pitr = std::ranges::find_if(
			presentModes,
			[](const vk::PresentModeKHR& pm) { return pm == vk::PresentModeKHR::eFifo; }
        );


        _capabilities = capabilities;
		_surfaceFormat = (fitr != formats.end()) ? *fitr : formats.at(0);
        _selectedPresentMode = (pitr != presentModes.end()) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eFifo; // currently we don't use mailbox

		CreateSwapChain();

        //
        // sync objects
        //
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        //vk::FenceCreateInfo fenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
        for (auto i = 0u; i < _surfaceImages.size(); ++i)
        {
            _imageReadySemapores.emplace_back(deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo));

        }
      
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        _desc.width = width;
        _desc.height = height;
        _capabilities = _deviceContext.PhysicalDevice()->getSurfaceCapabilitiesKHR(*_desc.surface);
        _currentSemaphore = 0;

        CreateSwapChain();
    }

    SwapChainImageReference SwapChain::AcquireNextAvailableImageAsync()
    {
        // https://stackoverflow.com/questions/65054157/do-i-need-dedicated-fences-semaphores-per-swap-chain-image-per-frame-or-per-com
        _currentSemaphore = (_currentSemaphore + 1) % _imageReadySemapores.size();

        auto semaphoreHandle = *_imageReadySemapores[_currentSemaphore];

        auto [result, index] = _swapchain.acquireNextImage(UINT64_MAX, semaphoreHandle);

        _lastAquiredImage = index;
        VK_CHECK(result);

        return SwapChainImageReference
        {
            .image_index = index,
            .image_ready = semaphoreHandle
        };
    }

    vk::Result SwapChain::PresentLastAcquiredImageAsync(vk::Semaphore renderingDoneSemaphore)
    {
        //VkSemaphore sem = renderingDoneSemaphore;
        vk::PresentInfoKHR presentInfo
        {
             .waitSemaphoreCount = 1,
             .pWaitSemaphores = &renderingDoneSemaphore,
             .swapchainCount = 1,
             .pSwapchains = &*_swapchain,
             .pImageIndices = &_lastAquiredImage
        };
        return _presentationQueue.presentKHR(presentInfo);
    }

    vk::Rect2D SwapChain::RenderArea() const
    {
        return vk::Rect2D
        {
            {0u, 0u},
            {_desc.width, _desc.height}
        };
    }

    std::vector<std::shared_ptr<Image>> SwapChain::Images() const
    {
        return _surfaceImages;
    }

    void SwapChain::CreateSwapChain()
    {
        vk::Extent2D selectedExtent = CalcMaxExtent(_desc.width, _desc.height, _capabilities);
        uint32_t minImageCount = std::max(2u, _capabilities.minImageCount);
        vk::SwapchainCreateInfoKHR createInfo
        {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = *_desc.surface,
            .minImageCount = minImageCount,
            .imageFormat = _surfaceFormat.format,
            .imageColorSpace = _surfaceFormat.colorSpace,
            .imageExtent = selectedExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .preTransform = _capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = _selectedPresentMode,
            .clipped = VK_TRUE
        };

        std::array<uint32_t, 2> queueFamiliyIndices{ _desc.graphics_queue_family, _desc.present_queue_family };
        if (_desc.graphics_queue_family != _desc.present_queue_family)
        {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamiliyIndices.data();
        }
        else
        {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }
        _swapchain = nullptr; // first release then create
        _swapchain = _deviceContext.LogicalDevice()->createSwapchainKHR(createInfo);
        auto images = _swapchain.getImages();

        _surfaceImages.resize(images.size(), nullptr);

        for (auto i = 0u; i < images.size(); ++i)
        {
            vk::ImageSubresourceRange subResourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1, // no mip map for now
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            vk::ImageViewCreateInfo imageViewCreateInfo
            {
                .flags = vk::ImageViewCreateFlags(),
                .image = images[i],
                .viewType = vk::ImageViewType::e2D,
                .format = _surfaceFormat.format,
                .components = vk::ComponentMapping(),
                .subresourceRange = subResourceRange
            };

            auto view = _deviceContext.LogicalDevice()->createImageView(imageViewCreateInfo);
            _surfaceImages[i] = std::make_shared<Image>(images[i], std::move(view));
        }
    }

    uint32_t SwapChain::ImageCount() const
    {
        return static_cast<uint32_t>(_surfaceImages.size());
    }


}