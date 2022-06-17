
#include <debugger_trace.hpp>
#include <trigger_debugger_breakpoint.hpp>

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
#if !EUREKA_VULKAN_VERBOSE
        if (messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
#endif        
        {
            static constexpr uint32_t chunksSize = 150;
            std::string_view all(pCallbackData->pMessage);
            std::string separated;
            separated.reserve(all.size() + 2 * (all.size() / chunksSize));

            auto prev = 0;
            for (auto i = chunksSize; i < all.size(); i += chunksSize)
            {
                while (i < all.size() && all[i] != ' ')
                {
                    ++i;
                }
                separated.append(pCallbackData->pMessage + prev, pCallbackData->pMessage + i);
                separated.append("\n");
                prev = i;
            }

            if (prev < all.size())
            {
                separated.append(pCallbackData->pMessage + prev, pCallbackData->pMessage + all.size());
            }

            DEBUGGER_TRACE("\n{}\n", separated);

            trigger_debugger_breakpoint();


        }
       

        return VK_FALSE;
    }

}
