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

    SwapChainTarget::SwapChainTarget(const GPURuntime& /*runtime*/, SwapChainTargetDesc desc) : _surface(std::move(desc.surface))
    {
		auto supportDetails = QuerySwapchainSupport(desc.physical_device, desc.surface);

    }

}