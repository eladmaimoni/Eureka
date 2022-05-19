#include "VkRuntime.hpp"
#include <string_view>
#include <debugger_trace.hpp>
#include "vk_error_handling.hpp"
#include <optional>

/*
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code

*/
namespace eureka
{
    void ValidateRequiredExtentionsExists(const VkRuntimeDesc& desc)
    {
        auto supported_extentions = vk::enumerateInstanceExtensionProperties();

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

    void ValidateRequiredLayersExists(const VkRuntimeDesc& desc)
    {
        auto supported_layers = vk::enumerateInstanceLayerProperties();

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

    void LogDeviceProperties(const vk::PhysicalDevice& device)
    {
        /*
        * void vkGetPhysicalDeviceProperties(
            VkPhysicalDevice                            physicalDevice,
            VkPhysicalDeviceProperties*                 pProperties);
        */

        vk::PhysicalDeviceProperties properties = device.getProperties();

        /*
        * typedef struct VkPhysicalDeviceProperties {
            uint32_t                            apiVersion;
            uint32_t                            driverVersion;
            uint32_t                            vendorID;
            uint32_t                            deviceID;
            VkPhysicalDeviceType                deviceType;
            char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
            uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
            VkPhysicalDeviceLimits              limits;
            VkPhysicalDeviceSparseProperties    sparseProperties;
            } VkPhysicalDeviceProperties;
        */
        std::string device_type;

        switch (properties.deviceType)
        {
        case (vk::PhysicalDeviceType::eCpu): device_type = "CPU"; break;
        case (vk::PhysicalDeviceType::eDiscreteGpu): device_type = "Discrete GPU"; break;
        case (vk::PhysicalDeviceType::eIntegratedGpu): device_type = "Integrated GPU"; break;
        case (vk::PhysicalDeviceType::eVirtualGpu):  device_type = "Virtual GPU"; break;
        default: device_type = "Other"; break;
        }

        DEBUGGER_TRACE("Device name: {} type: {}", properties.deviceName, device_type);
    }

    bool IsDeviceSuitableForDisplay(const vk::PhysicalDevice& device)
    {
        /*
        * A device is suitable if it can present to the screen, ie support
        * the swapchain extension
        */
        const std::vector<std::string_view> requestedExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        auto availableExtentions = device.enumerateDeviceExtensionProperties();

        for (const auto& requestedExtension : requestedExtensions)
        {
            bool found = false;
            for (auto availableExtention : availableExtentions)
            {
                std::string_view availableExtentionName(availableExtention.extensionName);
                if (requestedExtension == availableExtentionName)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                return false;
            }
        }

        return true;
    }


    struct QueueFamilies
    {
        std::optional<uint32_t> direct_graphics;
        std::optional<uint32_t> copy;
        std::optional<uint32_t> compute;
    };

    QueueFamilies QueryAvailableQueueFamilies(const vk::PhysicalDevice& device)
    {
        QueueFamilies families;

        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
    
        uint32_t i = 0u;
        for (const auto& queueFamilyProperties : queueFamilies)
        {
            auto copy     = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer;
            auto compute  = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute;
            auto graphics = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics;


            if (!families.direct_graphics && copy && compute && graphics)
            {
                families.direct_graphics = i;
            }
            else if (!families.copy && copy && (!compute) && (!graphics))
            {
                families.copy = i;
            }
            else if (!families.compute && compute && (!graphics))
            {
                families.compute = i;
            }
            ++i;
        }

        return families;
    }

    VkRuntime::VkRuntime(const VkRuntimeDesc& desc)
    {
        InitInstance(desc);

        InitDebugMessenger();

        InitDeviceAndQueues(desc);

      

    }

    VkRuntime::~VkRuntime()
    {
        
    }

    void VkRuntime::InitInstance(const VkRuntimeDesc& desc)
    {
        uint32_t version;
        VK_CHECK(vk::enumerateInstanceVersion(&version));

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

        ValidateRequiredExtentionsExists(desc);
        ValidateRequiredLayersExists(desc);
        
        vk::InstanceCreateInfo createInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()), 
            .ppEnabledLayerNames = desc.required_layers.data(), // enabled layers
            .enabledExtensionCount = static_cast<uint32_t>(desc.required_instance_extentions.size()),
            .ppEnabledExtensionNames = desc.required_instance_extentions.data()
        };

        _instance = vk::createInstance(createInfo);
    }



    void VkRuntime::InitDebugMessenger()
    {
        _loader = vk::DispatchLoaderDynamic(_instance, vkGetInstanceProcAddr);

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{
            .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = VkDebugMessengerCallback,
            .pUserData = nullptr
        };
        _messanger = _instance.createDebugUtilsMessengerEXT(createInfo, nullptr, _loader);
    }


    void VkRuntime::InitDeviceAndQueues(const VkRuntimeDesc& desc)
    {
        /*
        * Vulkan separates the concept of physical and logical devices.
        *
          A physical device usually represents a single complete implementation of Vulkan
          (excluding instance-level functionality) available to the host,
          of which there are a finite number.

          A logical device represents an instance of that implementation
          with its own state and resources independent of other logical devices.

         */

        std::vector<vk::PhysicalDevice> availableDevices = _instance.enumeratePhysicalDevices();

        vk::PhysicalDevice chosenPhysicalDevice;
        bool found = false;
        for (vk::PhysicalDevice device : availableDevices)
        {
            LogDeviceProperties(device);

            if (IsDeviceSuitableForDisplay(device))
            {
                chosenPhysicalDevice = device;
                found = true;
                break;

            }
        }

        if (!found)
        {
            DEBUGGER_TRACE("no suitable vulkan device found");
            throw vk::SystemError(vk::Result::eErrorUnknown);
        }

        auto queueFamilies = QueryAvailableQueueFamilies(chosenPhysicalDevice);







        std::array<float, 3> queuePriorities{ 1.0f, 1.0f, 1.0f };
        if (queueFamilies.direct_graphics && queueFamilies.compute && queueFamilies.copy)
        {
            // create 3 queues from different families
            std::array<vk::DeviceQueueCreateInfo, 3> queueCreateInfos;

            queueCreateInfos[0] = vk::DeviceQueueCreateInfo{
                .flags = vk::DeviceQueueCreateFlags{},
                .queueFamilyIndex = *queueFamilies.direct_graphics,
                .queueCount = 1,
                .pQueuePriorities = &queuePriorities[0]
            };
            queueCreateInfos[1] = vk::DeviceQueueCreateInfo{
                .flags = vk::DeviceQueueCreateFlags{},
                .queueFamilyIndex = *queueFamilies.compute,
                .queueCount = 1,
                .pQueuePriorities = &queuePriorities[1]
            };

            queueCreateInfos[2] = vk::DeviceQueueCreateInfo{
                .flags = vk::DeviceQueueCreateFlags{},
                .queueFamilyIndex = *queueFamilies.copy,
                .queueCount = 1,
                .pQueuePriorities = &queuePriorities[2]
            };

            vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();



            vk::DeviceCreateInfo deviceInfo{
                .flags = vk::DeviceCreateFlags(),
                .queueCreateInfoCount = 3,
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
                .ppEnabledLayerNames = desc.required_layers.data(),
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = &deviceFeatures
            };


            _device = chosenPhysicalDevice.createDevice(deviceInfo);

            _graphicsQueue = _device.getQueue(*queueFamilies.direct_graphics, 0);
            _computeQueue = _device.getQueue(*queueFamilies.compute, 0);
            _copyQueue = _device.getQueue(*queueFamilies.copy, 0);
        }
        else
        {
            // create 3 queues from the same family
            vk::DeviceQueueCreateInfo queueCreateInfo{
                .flags = vk::DeviceQueueCreateFlags(),
                .queueFamilyIndex = *queueFamilies.direct_graphics,
                .queueCount = 3,
                .pQueuePriorities = queuePriorities.data()
            };

            vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
 
            vk::DeviceCreateInfo deviceInfo{
                .flags = vk::DeviceCreateFlags(),
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &queueCreateInfo,
                .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
                .ppEnabledLayerNames = desc.required_layers.data(),
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = &deviceFeatures
            };

            _device = chosenPhysicalDevice.createDevice(deviceInfo);

            _graphicsQueue = _device.getQueue(*queueFamilies.direct_graphics, 0);
            _computeQueue = _device.getQueue(*queueFamilies.compute, 1);
            _copyQueue = _device.getQueue(*queueFamilies.copy, 2);

            DEBUGGER_TRACE("warning: created 3 queues from the same family");
        }
    }
}