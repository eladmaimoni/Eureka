#include "Instance.hpp"
#include "VkHelpers.hpp"
#include <debugger_trace.hpp>
#include "vk_error_handling.hpp"

/*
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code

*/
namespace eureka
{
    void ValidateRequiredExtentionsExists(const vkr::Context& context, const InstanceConfig& desc)
    {
        auto supported_extentions = context.enumerateInstanceExtensionProperties();

        //DEBUGGER_TRACE("Available instance extentions: \n{}", supported_extentions | std::views::transform([](const auto& v) {return std::string_view(v.extensionName); }) );


        for (auto required_extention : desc.required_instance_extentions)
        {
            bool found = false;
            for (const auto& supported_extention : supported_extentions)
            {
                std::string_view supported_extention_name(supported_extention.extensionName.data());
                if (supported_extention_name == required_extention)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw vk::SystemError(vk::Result::eErrorExtensionNotPresent);
            }
        }
    }

    void ValidateRequiredLayersExists(const vkr::Context& context, const InstanceConfig& desc)
    {
        auto supported_layers = context.enumerateInstanceLayerProperties();


        //DEBUGGER_TRACE("Available instance layers: \n{}", supported_layers | std::views::transform([](const auto& v) {return std::string_view(v.layerName.data()); }));


        for (const auto& required_layer : desc.required_layers)
        {
            bool found = false;
            for (const auto& supported_layer : supported_layers)
            {
                std::string_view supported_layer_name(supported_layer.layerName.data());
                if (supported_layer_name == required_layer)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw vk::SystemError(vk::Result::eErrorLayerNotPresent);
            }
        }
    }

    std::pair<vkr::Instance, uint32_t> InitInstance(const vkr::Context& context, const InstanceConfig& desc)
    {
        uint32_t version = context.enumerateInstanceVersion();

        DEBUGGER_TRACE("vulkan api version {}.{}.{}",
            VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version),
            VK_API_VERSION_PATCH(version)
        );

        vk::ApplicationInfo appInfo{
            .pApplicationName = "Eureka",
            .applicationVersion = version,
            .pEngineName = "Eureka Engine",
            .engineVersion = version,
            .apiVersion = version
        };

        ValidateRequiredExtentionsExists(context, desc);
        ValidateRequiredLayersExists(context, desc);


        DEBUGGER_TRACE("Requested instance layers = {}", desc.required_layers);
        DEBUGGER_TRACE("Requested instance extensions  = {}", desc.required_instance_extentions);

        vk::InstanceCreateInfo createInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
            .ppEnabledLayerNames = desc.required_layers.data(), // enabled layers
            .enabledExtensionCount = static_cast<uint32_t>(desc.required_instance_extentions.size()),
            .ppEnabledExtensionNames = desc.required_instance_extentions.data()
        };

        return std::make_pair(vkr::Instance(context, createInfo), version);
    }
    vkr::DebugUtilsMessengerEXT InitDebugMessenger(const vkr::Instance& instance)
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = VkDebugMessengerCallback,
        .pUserData = nullptr
        };

        return vkr::DebugUtilsMessengerEXT(instance, createInfo);

    }


    Instance::Instance(const InstanceConfig& desc)
    {
        std::tie(_instance, _vulkanApiVersion) = InitInstance(_context, desc);
        _debugMessenger = InitDebugMessenger(_instance);
    }

    Instance::~Instance()
    {
        
    }









}