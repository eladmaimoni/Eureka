
#include <vulkan/vulkan.hpp>
namespace eureka
{
    void CheckVulkan(vk::Result res, const char* stmt, const char* fname, int line);
    void CheckVulkan(vk::Result res);

    VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* userData
    );
}

#ifndef VK_CHECK
#ifdef NDEBUG
#define VK_CHECK(stmt) eureka::CheckVulkan(stmt); 
#else
#define VK_CHECK(stmt) eureka::CheckVulkan(stmt, #stmt, __FILE__, __LINE__); 
#endif
#endif
