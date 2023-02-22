#include "Instance.hpp"
#include "Result.hpp"
#include <algorithm>
#include <debugger_trace.hpp>
#include <ranges>

//#define VK_MAKE_API_VERSION(variant, major, minor, patch)                                                              \
//    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

//#define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
//#define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
//#define VK_API_VERSION_PATCH(version) ((uint32_t)(version)&0xFFFU)

namespace eureka::vulkan
{
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void* /*userData*/
    )
    {

#if !EUREKA_VULKAN_VERBOSE
        if(messageSeverity &
           (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
#endif
        {
            static constexpr uint32_t CHUNK_SIZE = 150;
            std::string_view all(pCallbackData->pMessage);
            std::string separated;
            separated.reserve(all.size() + 2 * (all.size() / CHUNK_SIZE));

            auto prev = 0;
            for(auto i = CHUNK_SIZE; i < all.size(); i += CHUNK_SIZE)
            {
                while(i < all.size() && all[i] != ' ')
                {
                    ++i;
                }
                separated.append(pCallbackData->pMessage + prev, pCallbackData->pMessage + i);
                separated.append("\n");
                prev = i;
            }

            if(prev < all.size())
            {
                separated.append(pCallbackData->pMessage + prev, pCallbackData->pMessage + all.size());
            }

            DEBUGGER_TRACE("\n{}\n", separated);
            //DEBUGGER_TRACE("{}", pCallbackData->pMessage);

            //trigger_debugger_breakpoint();
        }

        return VK_FALSE;
    }

    void ValidateRequiredExtentionsExists(const InstanceConfig& desc)
    {
        uint32_t propertyCount = 0;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr));

        std::vector<VkExtensionProperties> supportedExtentions(propertyCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, supportedExtentions.data()));
        //for (const auto& supportedExtention : supportedExtentions)
        //{
        //    std::string_view supported_extention_name(supportedExtention.extensionName);
        //    DEBUGGER_TRACE("instance extention = {}", supported_extention_name);
        //}
        for(auto requiredExtention : desc.required_instance_extentions)
        {

            bool found = false;
            for(const auto& supportedExtention : supportedExtentions)
            {
                std::string_view supported_extention_name(supportedExtention.extensionName);
                //DEBUGGER_TRACE("looking for {}, candidate = {}", requiredExtention, supported_extention_name);
                if(supported_extention_name == requiredExtention)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                throw ResultError(VkResult::VK_ERROR_EXTENSION_NOT_PRESENT, requiredExtention);
            }
        }
    }

    void ValidateRequiredLayersExists(const InstanceConfig& desc)
    {
        uint32_t propertyCount = 0;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr));

        std::vector<VkLayerProperties> layerProperties(propertyCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data()));
        //for (const auto& supportedLayer : layerProperties)
        //{
        //    std::string_view supported_layer_name(supportedLayer.layerName);
        //    DEBUGGER_TRACE("instance layer = {}", supported_layer_name);
        //}
        for(const auto& requiredLayer : desc.required_layers)
        {
            bool found = false;
            for(const auto& supportedLayer : layerProperties)
            {
                std::string_view supported_layer_name(supportedLayer.layerName);
                if(supported_layer_name == requiredLayer)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                throw ResultError(VkResult::VK_ERROR_LAYER_NOT_PRESENT, requiredLayer);
            }
        }
    }

    Instance::Instance(const InstanceConfig& config) :
        _config(config)
    {
        VK_CHECK(volkInitialize());

        uint32_t apiVersion{};
        VK_CHECK(vkEnumerateInstanceVersion(&apiVersion));
        _apiVersion = Version(apiVersion);

        DEBUGGER_TRACE("vulkan instance api version {}.{}.{}",
                       _apiVersion.Major(),
                       _apiVersion.Minor(),
                       _apiVersion.Patch());

        ValidateRequiredExtentionsExists(config);
        ValidateRequiredLayersExists(config);

        if(config.version)
        {
            DEBUGGER_TRACE("THIS IS WRONG, instance API is sdk version, not the supported api version");
            auto version = *config.version;
            _apiVersion = version;
        }

        VkApplicationInfo appInfo {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Eureka",
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName = "Eureka Engine",
            .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3, // the application is designed to work with versions 1.2 up to 1.3
        };
        

        
        VkInstanceCreateInfo createInfo {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = {},
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(_config.required_layers.size()),
            .ppEnabledLayerNames = _config.required_layers.data(), // enabled layers
            .enabledExtensionCount = static_cast<uint32_t>(_config.required_instance_extentions.size()),
            .ppEnabledExtensionNames = _config.required_instance_extentions.data(),
        };

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));

        volkLoadInstance(_instance);

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessangerCreateInfo {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .flags = VkDebugUtilsMessengerCreateFlagsEXT {},
            .messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugMessengerCallback,
            .pUserData = nullptr};

        auto debugUtilsExt = std::string_view(INSTANCE_EXTENTION_DEBUG_UTILS);
        if(config.required_instance_extentions.end() !=
           std::ranges::find_if(config.required_instance_extentions,
                                [debugUtilsExt](const char* ext) { return debugUtilsExt == ext; }))
        {
            VK_CHECK(
                vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsMessangerCreateInfo, nullptr, &_debugMessenger));
        }
    }

    Instance::~Instance()
    {
        if(_debugMessenger)
        {
            vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }

        if(_instance)
        {
            vkDestroyInstance(_instance, nullptr);
        }
    }

    std::vector<VkPhysicalDevice> Instance::EnumeratePhysicalDevices()
    {
        uint32_t physicalDeviceCount = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr));

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data()));

        return physicalDevices;
    }

    Version Instance::ApiVersion() const
    {
        return _apiVersion;
    }

    void Instance::DestroySurface(VkSurfaceKHR surface) const
    {
        vkDestroySurfaceKHR(_instance, surface, nullptr);
    }


    std::shared_ptr<Instance> MakeDefaultInstance(std::optional<Version> version)
    {
        InstanceConfig config {};
        config.required_instance_extentions.emplace_back(INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME);
#ifdef WIN32
        config.required_instance_extentions.emplace_back(INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME);
#endif // WIN32

#ifndef NDEBUG
        config.required_instance_extentions.emplace_back(INSTANCE_EXTENTION_DEBUG_UTILS);
        config.required_layers.emplace_back(INSTANCE_LAYER_VALIDATION);
#endif
        //config.version = version;
        //if (config.version)
        //{
        //    if (config.version->Minor() <= 2)
        //    {
        //        config.required_instance_extentions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        //        config.required_instance_extentions.emplace_back("VK_KHR_get_surface_capabilities2");
        //        config.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
        //    }
        //}
        return std::make_shared<Instance>(config);
    }

    dspan<const char*> Instance::EnabledExtentions() const
    {
        // that const cast is OK I guess
        auto ptr = const_cast<const char**>(_config.required_instance_extentions.data());
        return dspan<const char*>(ptr, _config.required_instance_extentions.size());
    }
} // namespace eureka::vulkan
