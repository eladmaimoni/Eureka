#pragma once

#include <vulkan/vulkan_raii.hpp>

/*
https://www.youtube.com/watch?v=ErtSXzVG7nU
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
*/
namespace eureka
{
    inline constexpr char VK_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";

    struct InstanceConfig
    {
        std::vector<const char*> required_instance_extentions;
        std::vector<const char*> required_layers;
    };

    class Instance
    {
    private:
        vkr::Context _context;
        vkr::Instance _instance{nullptr};
        vkr::DebugUtilsMessengerEXT _debugMessenger{nullptr};

        uint32_t _vulkanApiVersion{};


    public:
        Instance(const InstanceConfig& desc);
        ~Instance();
        uint32_t ApiVersion() const { return _vulkanApiVersion; }
        const vkr::Instance& Get() const { return _instance; }

    };



}

/*
Note about Dispatcher.

by default, each call to a method in Vulkan-Hpp takes a Dispatch template argument.

somehow, without providing the template type or the Dispatch argument, the API
passes a default DispatchLoaderBase implementation.
this implementation is the DispatchLoaderStatic that has methods to call all (?) vulkan c api 
which are statically linked.

When using the raii::Context together with other raii objects, we no longer have the dispatch parameter.
instead, the context itself instantiates a DispatchLoaderBase parameter that loads the API dynamically.

This means we do not need to link against Vulkan static library?


this Dispatch object is statically created and calls vulkan methods that are statically
linked to the vulkan-1.lib or something.

if you use
*/