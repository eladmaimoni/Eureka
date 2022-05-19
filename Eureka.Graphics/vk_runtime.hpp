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
    inline constexpr char VALIDATION_LAYER_NAME[] = "VK_LAYER_KHRONOS_validation";

    struct VkRuntimeDesc
    {
        std::vector<const char*> required_instance_extentions;
        std::vector<const char*> required_layers;

    };

    class VkRuntime
    {
        vk::Instance _instance;
    public:
        VkRuntime(const VkRuntimeDesc& desc)
        {
            InitInstance(desc);
        }

        void InitInstance(const VkRuntimeDesc& desc);

    private:

    };
}