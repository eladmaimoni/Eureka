#include "vk_runtime.hpp"
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
    
        auto directQueueDesiredFlags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;
        auto copyQueueDesiredFlags = vk::QueueFlagBits::eTransfer;
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

        auto appInfo = vk::ApplicationInfo(
            "Eureka",
            version,
            "Eureka Engine",
            version,
            version
        );

        ValidateRequiredExtentionsExists(desc);
        ValidateRequiredLayersExists(desc);
        
        //vk::DebugUtilsMessangerEXT;

        auto createInfo = vk::InstanceCreateInfo(
            vk::InstanceCreateFlags(),
            &appInfo,
            static_cast<uint32_t>(desc.required_layers.size()), desc.required_layers.data(), // enabled layers
            static_cast<uint32_t>(desc.required_instance_extentions.size()),
            desc.required_instance_extentions.data()
        );

        _instance = vk::createInstance(createInfo);
    }



    void VkRuntime::InitDebugMessenger()
    {
        _loader = vk::DispatchLoaderDynamic(_instance, vkGetInstanceProcAddr);


        vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            VkDebugMessengerCallback,
            nullptr
        );
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
            queueCreateInfos[0] = vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags(),
                *queueFamilies.direct_graphics,
                1,
                &queuePriorities[0]
            );

            queueCreateInfos[1] = vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags(),
                *queueFamilies.compute,
                1,
                &queuePriorities[1]
            );
            queueCreateInfos[2] = vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags(),
                *queueFamilies.copy,
                1,
                &queuePriorities[2]
            );

            vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();

            vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
                vk::DeviceCreateFlags(),
                3, queueCreateInfos.data(),
                static_cast<uint32_t>(desc.required_layers.size()),
                desc.required_layers.data(),
                0, nullptr,
                &deviceFeatures
            );


            _device = chosenPhysicalDevice.createDevice(deviceInfo);

            _graphicsQueue = _device.getQueue(*queueFamilies.direct_graphics, 0);
            _computeQueue = _device.getQueue(*queueFamilies.compute, 0);
            _copyQueue = _device.getQueue(*queueFamilies.copy, 0);
        }
        else
        {
            // create 3 queues from the same family

            auto queueCreateInfo = vk::DeviceQueueCreateInfo(
                vk::DeviceQueueCreateFlags(),
                *queueFamilies.direct_graphics,
                3,
                queuePriorities.data()
            );

            vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();

            vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
                vk::DeviceCreateFlags(),
                1, &queueCreateInfo,
                static_cast<uint32_t>(desc.required_layers.size()),
                desc.required_layers.data(),
                0, nullptr,
                &deviceFeatures
            );

            _device = chosenPhysicalDevice.createDevice(deviceInfo);

            _graphicsQueue = _device.getQueue(*queueFamilies.direct_graphics, 0);
            _computeQueue = _device.getQueue(*queueFamilies.compute, 1);
            _copyQueue = _device.getQueue(*queueFamilies.copy, 2);

            DEBUGGER_TRACE("warning: created 3 queues from the same family");
        }
    }
}