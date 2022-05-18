
#include <vulkan/vulkan.hpp>
namespace eureka
{

    void CheckVulkan(vk::Result res, const char* stmt, const char* fname, int line);
    void CheckVulkan(vk::Result res);
}

#ifndef VK_CHECK
#ifdef NDEBUG
#define VK_CHECK(stmt) eureka::CheckVulkan(stmt); 
#else
#define VK_CHECK(stmt) eureka::CheckVulkan(stmt, #stmt, __FILE__, __LINE__); 
#endif
#endif
