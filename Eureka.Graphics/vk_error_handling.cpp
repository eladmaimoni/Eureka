#include "VkRuntime.hpp"
#include <debugger_trace.hpp>

namespace eureka
{
    void CheckVulkan(vk::Result res, const char* stmt, const char* fname, int line)
    {
        if (res != vk::Result::eSuccess)
        {
            auto str = formatted_debugger_line(stmt, fname, line, "vulkan api error");
            debug_output(str);
            vk::throwResultException(res, "vulkan api error");
        }
    }


    void CheckVulkan(vk::Result res)
    {
        if (res != vk::Result::eSuccess)
        {
            vk::throwResultException(res, "vulkan api error");
        }
    }

	/*
* Debug call back:
*
*	typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
	} VkDebugUtilsMessageSeverityFlagBitsEXT;
*	typedef enum VkDebugUtilsMessageTypeFlagBitsEXT {
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
		VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
	} VkDebugUtilsMessageTypeFlagBitsEXT;
*	typedef struct VkDebugUtilsMessengerCallbackDataEXT {
		VkStructureType                              sType;
		const void*                                  pNext;
		VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
		const char*                                  pMessageIdName;
		int32_t                                      messageIdNumber;
		const char*                                  pMessage;
		uint32_t                                     queueLabelCount;
		const VkDebugUtilsLabelEXT*                  pQueueLabels;
		uint32_t                                     cmdBufLabelCount;
		const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
		uint32_t                                     objectCount;
		const VkDebugUtilsObjectNameInfoEXT*         pObjects;
	} VkDebugUtilsMessengerCallbackDataEXT;
*/

    VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* /*userData*/
    )
    {

        if (messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
        {
            DEBUGGER_TRACE("validation layer: {}", pCallbackData->pMessage);
        }
       

        return VK_FALSE;
    }

}
