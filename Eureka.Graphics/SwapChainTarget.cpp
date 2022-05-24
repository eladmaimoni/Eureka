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

		/*
		* typedef struct VkSurfaceCapabilitiesKHR {
			uint32_t                         minImageCount;
			uint32_t                         maxImageCount;
			VkExtent2D                       currentExtent;
			VkExtent2D                       minImageExtent;
			VkExtent2D                       maxImageExtent;
			uint32_t                         maxImageArrayLayers;
			VkSurfaceTransformFlagsKHR       supportedTransforms;
			VkSurfaceTransformFlagBitsKHR    currentTransform;
			VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
			VkImageUsageFlags                supportedUsageFlags;
		} VkSurfaceCapabilitiesKHR;
		*/

		support.capabilities = device->getSurfaceCapabilitiesKHR(surface);

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
			)", 
			support.capabilities.minImageCount,
			support.capabilities.maxImageCount,
			support.capabilities.currentExtent.width, support.capabilities.currentExtent.height,
			support.capabilities.minImageExtent.width, support.capabilities.minImageExtent.height,
            support.capabilities.maxImageExtent.width, support.capabilities.maxImageExtent.height,
			support.capabilities.maxImageArrayLayers,
			support.capabilities.supportedTransforms
		);

		std::vector< vk::SurfaceTransformFlagBitsKHR> val;


        
		//auto tmp = ;





		//support.capabilities.supportedTransforms
		//	std::cout << "\tsupported transforms:\n";
		//	std::vector<std::string> stringList = log_transform_bits();
		//	for (std::string line : stringList) {
		//		std::cout << "\t\t" << line << '\n';
		//	}


		//if (debug) {


		//	std::cout << "\tcurrent extent: \n";
		//	/*typedef struct VkExtent2D {
		//		uint32_t    width;
		//		uint32_t    height;
		//	} VkExtent2D;
		//	*/

		//	std::cout << "\tcurrent transform:\n";
		//	stringList = log_transform_bits(support.capabilities.currentTransform);
		//	for (std::string line : stringList) {
		//		std::cout << "\t\t" << line << '\n';
		//	}

		//	std::cout << "\tsupported alpha operations:\n";
		//	stringList = log_alpha_composite_bits(support.capabilities.supportedCompositeAlpha);
		//	for (std::string line : stringList) {
		//		std::cout << "\t\t" << line << '\n';
		//	}

		//	std::cout << "\tsupported image usage:\n";
		//	stringList = log_image_usage_bits(support.capabilities.supportedUsageFlags);
		//	for (std::string line : stringList) {
		//		std::cout << "\t\t" << line << '\n';
		//	}
		//}

		//support.formats = device.getSurfaceFormatsKHR(surface);

		//if (debug) {

		//	for (vk::SurfaceFormatKHR supportedFormat : support.formats) {
		//		/*
		//		* typedef struct VkSurfaceFormatKHR {
		//			VkFormat           format;
		//			VkColorSpaceKHR    colorSpace;
		//		} VkSurfaceFormatKHR;
		//		*/

		//		std::cout << "supported pixel format: " << vk::to_string(supportedFormat.format) << '\n';
		//		std::cout << "supported color space: " << vk::to_string(supportedFormat.colorSpace) << '\n';
		//	}
		//}

		//support.presentModes = device.getSurfacePresentModesKHR(surface);

		//for (vk::PresentModeKHR presentMode : support.presentModes) {
		//	std::cout << '\t' << log_present_mode(presentMode) << '\n';
		//}
		return support;
	}

    SwapChainTarget::SwapChainTarget(const GPURuntime& /*runtime*/, SwapChainTargetDesc desc) : _surface(std::move(desc.surface))
    {
		auto supportDetails = QuerySwapchainSupport(desc.physical_device, desc.surface);

    }

}