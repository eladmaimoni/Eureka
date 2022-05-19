#pragma once

#include <vulkan/vulkan.hpp>


/*
https://www.youtube.com/watch?v=ErtSXzVG7nU
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
*/
namespace eureka
{
    inline constexpr char VK_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";
    inline constexpr char VK_EXT_DEBUG_UTILS[] = "VK_EXT_debug_utils";


    struct VkRuntimeDesc
    {
        std::vector<const char*> required_instance_extentions;
        std::vector<const char*> required_layers;

    };

    class VkRuntime
    {
    public:
        vk::Instance _instance;
        vk::DispatchLoaderDynamic _loader;
        vk::DebugUtilsMessengerEXT _messanger;

        

        vk::Device _device;
        vk::Queue _graphicsQueue;
        vk::Queue _computeQueue;
        vk::Queue _copyQueue;


    public:
        
        VkRuntime(const VkRuntimeDesc& desc);
        ~VkRuntime();
    private:
        void InitInstance(const VkRuntimeDesc& desc);
        void InitDebugMessenger();
        void InitDeviceAndQueues(const VkRuntimeDesc& desc);
    };
}