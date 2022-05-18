#pragma once

#include <string_view>
#include <debugger_trace.hpp>
#include "vk_error_handling.hpp"

/*
https://www.youtube.com/watch?v=ErtSXzVG7nU
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
*/
namespace eureka
{
    struct VkRuntimeDesc
    {
        std::vector<const char*> required_instance_extentions;
    };

    class VkRuntime
    {
        vk::Instance _instance;
    public:
        VkRuntime(const VkRuntimeDesc& desc)
        {
            InitInstance(desc);
        }

        void InitInstance(const VkRuntimeDesc& desc)
        {

            uint32_t version;
            VK_CHECK(vk::enumerateInstanceVersion(&version));

  
            DEBUGGER_TRACE("vulkan api version {}.{}.{}", 
                VK_API_VERSION_MAJOR(version),
                VK_API_VERSION_MINOR(version),
                VK_API_VERSION_PATCH(version)            
            );

            auto appInfo = vk::ApplicationInfo(
                "Eureka",
                version,
                "Eureka Engine",
                version,
                version
            );
            
            auto createInfo = vk::InstanceCreateInfo(
                vk::InstanceCreateFlags(),
                &appInfo,
                0, nullptr, // enabled layers
                static_cast<uint32_t>(desc.required_instance_extentions.size()),
                desc.required_instance_extentions.data()
            );

            _instance = vk::createInstance(createInfo);

        }

    };
}