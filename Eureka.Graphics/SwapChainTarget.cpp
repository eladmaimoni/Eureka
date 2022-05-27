#include "SwapChainTarget.hpp"
#include <debugger_trace.hpp>

namespace eureka
{
    template<typename Object>
    concept has_vk_to_string = requires(const Object & obj)
    {
        vk::to_string(obj);
    };

	inline std::string to_string(const has_vk_to_string auto& value)
	{
		return vk::to_string(value);
	}

	static_assert(!has_eureka_to_string<std::string>);
	static_assert(has_vk_to_string<vk::PhysicalDeviceType>);
	static_assert(has_eureka_to_string<vk::PhysicalDeviceType>);
    static_assert(iterable_of_to_stringable<std::vector< vk::SurfaceTransformFlagBitsKHR>>);
    static_assert(!iterable_of_streamable<std::vector< vk::SurfaceTransformFlagBitsKHR>>);

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
		const vk::raii::PhysicalDevice* device, 
		vk::SurfaceKHR surface) 
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

    SwapChainTarget::SwapChainTarget(const GPURuntime& /*runtime*/, SwapChainTargetDesc desc) 
		: _surface(std::move(desc.surface))
    {
		auto [capabilities, formats, presentModes] = QuerySwapchainSupport(desc.physical_device, *_surface);

		auto fitr = std::ranges::find_if(
            formats,
			[](const vk::SurfaceFormatKHR& sf) { return sf.format == vk::Format::eB8G8R8A8Unorm && sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; }
		);


        auto pitr = std::ranges::find_if(
			presentModes,
			[](const vk::PresentModeKHR& pm) { return pm == vk::PresentModeKHR::eMailbox; }
        );

		vk::SurfaceFormatKHR selectedFormat = (fitr != formats.end()) ? *fitr : formats.at(0);
        vk::PresentModeKHR selectedPresentMode = (pitr != presentModes.end()) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
		vk::Extent2D selectedExtent = CalcMaxExtent(desc.width, desc.height, capabilities);


		uint32_t minImageCount = std::max(2u, capabilities.minImageCount);


		vk::SwapchainCreateInfoKHR createInfo
		{
			.flags = vk::SwapchainCreateFlagsKHR(),
			.surface = *_surface,
			.minImageCount = minImageCount,
			.imageFormat = selectedFormat.format,
			.imageColorSpace = selectedFormat.colorSpace,
			.imageExtent = selectedExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = selectedPresentMode,
			.clipped = VK_TRUE
		};

		std::array<uint32_t, 2> queueFamiliyIndices{ desc.graphics_queue_family, desc.present_queue_family };
		if (desc.graphics_queue_family != desc.present_queue_family)
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamiliyIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		_swapchain = desc.logical_device->createSwapchainKHR(createInfo);
		_images = _swapchain.getImages();
    
		std::vector<vk::raii::ImageView> _imageViews; 
		
		_imageViews.reserve(_images.size());
		
		for (auto& image : _images)
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
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = selectedFormat.format,
                .components = vk::ComponentMapping(),
                .subresourceRange = subResourceRange
			};

			_imageViews.emplace_back(
				desc.logical_device->createImageView(imageViewCreateInfo)
			);
		}
		


		//desc.logical_device->createImageView()
	}

}