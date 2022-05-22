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
    void ValidateRequiredExtentionsExists(const vk::raii::Context& context, const VkRuntimeDesc& desc)
    {
        auto supported_extentions = context.enumerateInstanceExtensionProperties();

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

    void ValidateRequiredLayersExists(const vk::raii::Context& context, const VkRuntimeDesc& desc)
    {
        auto supported_layers = context.enumerateInstanceLayerProperties();

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

    vk::raii::Instance InitInstance(const vk::raii::Context& context, const VkRuntimeDesc& desc)
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

        vk::InstanceCreateInfo createInfo{
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
            .ppEnabledLayerNames = desc.required_layers.data(), // enabled layers
            .enabledExtensionCount = static_cast<uint32_t>(desc.required_instance_extentions.size()),
            .ppEnabledExtensionNames = desc.required_instance_extentions.data()
        };

        return vk::raii::Instance(context, createInfo);
    }


    void LogDeviceProperties(const vk::raii::PhysicalDevice & device)
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
        //std::string device_type = ;

        //switch (properties.deviceType)
        //{
        //case (vk::PhysicalDeviceType::eCpu): device_type = "CPU"; break;
        //case (vk::PhysicalDeviceType::eDiscreteGpu): device_type = "Discrete GPU"; break;
        //case (vk::PhysicalDeviceType::eIntegratedGpu): device_type = "Integrated GPU"; break;
        //case (vk::PhysicalDeviceType::eVirtualGpu):  device_type = "Virtual GPU"; break;
        //default: device_type = "Other"; break;
        //}

        DEBUGGER_TRACE("Device name: {} type: {}", properties.deviceName, vk::to_string(properties.deviceType));
    }

    bool IsDeviceSuitableForDisplay(const vk::raii::PhysicalDevice& device)
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
        uint32_t                               direct_graphics_family_index;
        uint32_t                               present_family_index;
        uint32_t                               copy_family_index;
        uint32_t                               compute_family_index;

        uint32_t                               direct_graphics_create_index;
        uint32_t                               present_create_index;
        uint32_t                               copy_create_index;
        uint32_t                               compute_create_index;

        std::vector<std::vector<float>>        queue_priorities;
        std::vector<vk::DeviceQueueCreateInfo> create_info;
    };

    QueueFamilies QueryAvailableQueueFamilies(const vk::raii::PhysicalDevice& device, vk::SurfaceKHR presentationSurface)
    {

        QueueFamilies families;
        families.create_info.reserve(5);
        families.queue_priorities.reserve(5);

        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
    


        //
        // graphics queue
        // 
        uint32_t i = 0u;
        vk::DeviceQueueCreateInfo* graphicsCreateInfo{ nullptr };
        std::vector<float>* graphicsCreateInfoPriority{ nullptr };


        for (const auto& queueFamilyProperties : queueFamilies)
        {
            auto copy_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer;
            auto compute_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute;
            auto graphics = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics;

            if (graphics && compute_family_index && copy_family_index)
            {
                families.direct_graphics_family_index = i;
                families.direct_graphics_create_index = 0;

                graphicsCreateInfoPriority = &families.queue_priorities.emplace_back();
                auto priority = &graphicsCreateInfoPriority->emplace_back(1.0f);
                graphicsCreateInfo = &families.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = families.direct_graphics_family_index,
                        .queueCount = 1,
                        .pQueuePriorities = priority
                    }
                );
                break;
            }
            
            ++i;
        }

        if (!graphicsCreateInfo)
        {
            // no graphics queue
            throw vk::SystemError(vk::Result::eErrorUnknown);
        }

        //
        // compute queue
        //
        i = 0u;
        vk::DeviceQueueCreateInfo* computeCreateInfo{ nullptr };
        std::vector<float>* computeCreateInfoPriority{ nullptr };
        for (const auto& queueFamilyProperties : queueFamilies)
        {
            auto copy_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer;
            auto compute_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute;
            auto graphics = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics;

            if (i != families.direct_graphics_family_index && queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute)
            {
                families.compute_family_index = i;
                families.compute_create_index = 0;
                computeCreateInfoPriority = &families.queue_priorities.emplace_back();
                auto priority = &computeCreateInfoPriority->emplace_back(1.0f);
                computeCreateInfo = &families.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = families.compute_family_index,
                        .queueCount = 1,
                        .pQueuePriorities = priority
                    }
                );
                break;
            }
            ++i;
        }

        if (!computeCreateInfo)
        {
            families.compute_family_index = families.direct_graphics_family_index;
            computeCreateInfo = graphicsCreateInfo;
            families.compute_create_index = graphicsCreateInfo->queueCount++;
            graphicsCreateInfoPriority->emplace_back(1.0f);
            computeCreateInfoPriority = graphicsCreateInfoPriority;
        }


        //
        // copy queue
        //

        i = 0u;
        vk::DeviceQueueCreateInfo* copyCreateInfo{ nullptr };
        std::vector<float>* copyCreateInfoPriority{ nullptr };

        for (const auto& queueFamilyProperties : queueFamilies)
        {
            auto copy_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer;
            auto compute_family_index = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute;
            auto graphics = queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics;

            if (copy_family_index && !compute_family_index && !graphics)
            {
                families.copy_family_index = i;
                families.copy_create_index = 0;
                copyCreateInfoPriority = &families.queue_priorities.emplace_back();
                auto priority = &copyCreateInfoPriority->emplace_back(1.0f);

                copyCreateInfo = &families.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = families.copy_family_index,
                        .queueCount = 1,
                        .pQueuePriorities = priority
                    }
                );
                break;
            }
            ++i;
        }

        if (!copyCreateInfo && (queueFamilies[families.compute_family_index].queueFlags & vk::QueueFlagBits::eTransfer))
        {
            families.copy_family_index = families.compute_family_index;
            copyCreateInfo = computeCreateInfo;
            families.copy_create_index = computeCreateInfo->queueCount++;
            computeCreateInfoPriority->emplace_back(1.0f);
            copyCreateInfoPriority = computeCreateInfoPriority;
            copyCreateInfo->setPQueuePriorities(copyCreateInfoPriority->data());
        }

        if (!copyCreateInfo)
        {
            families.copy_family_index = families.direct_graphics_family_index;
            computeCreateInfo = graphicsCreateInfo;
            families.copy_create_index = graphicsCreateInfo->queueCount++; 
            graphicsCreateInfoPriority->emplace_back(1.0f);
            copyCreateInfoPriority = graphicsCreateInfoPriority;
            copyCreateInfo->setPQueuePriorities(copyCreateInfoPriority->data());
        }

        //
        // present queue
        //

   

        if(device.getSurfaceSupportKHR(families.copy_family_index, presentationSurface))
        {
            families.present_family_index = families.copy_family_index;
            families.present_create_index = copyCreateInfo->queueCount++;
            
            copyCreateInfoPriority->emplace_back(1.0f);
            copyCreateInfo->setPQueuePriorities(copyCreateInfoPriority->data());

        }
        else if (device.getSurfaceSupportKHR(families.compute_family_index, presentationSurface))
        {
            families.present_family_index = families.compute_family_index;
            families.present_create_index = computeCreateInfo->queueCount++;
            computeCreateInfoPriority->emplace_back(1.0f);
            computeCreateInfo->setPQueuePriorities(computeCreateInfoPriority->data());

        }
        else if (device.getSurfaceSupportKHR(families.direct_graphics_family_index, presentationSurface))
        {
            families.present_family_index = families.direct_graphics_family_index;
            families.present_create_index = graphicsCreateInfo->queueCount++;
            graphicsCreateInfoPriority->emplace_back(1.0f);
            graphicsCreateInfo->setPQueuePriorities(graphicsCreateInfoPriority->data());
        }
        else
        {
            bool found = false;
            for (i = 0u; i < queueFamilies.size(); ++i)
            {
                auto presentation = device.getSurfaceSupportKHR(i, presentationSurface);

                if (presentation)
                {
                    families.present_family_index = i;
                    families.present_create_index = 0;
                    auto priority = &families.queue_priorities.emplace_back().emplace_back(1.0f);
                    families.create_info.emplace_back(
                        vk::DeviceQueueCreateInfo{
                            .flags = vk::DeviceQueueCreateFlags{},
                            .queueFamilyIndex = families.present_family_index,
                            .queueCount = 1,
                            .pQueuePriorities = priority
                        }
                    );
                    break;
                }
            }

            if (!found)
            {
                // no present queue
                throw vk::SystemError(vk::Result::eErrorUnknown);
            }
        }
        return families;
    }

    vk::raii::DebugUtilsMessengerEXT InitDebugMessenger(const vk::raii::Instance& instance)
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = VkDebugMessengerCallback,
        .pUserData = nullptr
        };

        return vk::raii::DebugUtilsMessengerEXT(instance, createInfo);

    }


    VkRuntime::VkRuntime(const VkRuntimeDesc& desc)
        :
        _instance(InitInstance(_context, desc)),
        _debugMessenger(InitDebugMessenger(_instance))
    {
      
    }

    VkRuntime::~VkRuntime()
    {
        
    }







    VkDeviceContext::VkDeviceContext(const vk::raii::Instance& instance, const VkDeviceContextDesc& desc)
    {
        InitDeviceAndQueues(instance, desc);
    }

    VkDeviceContext::~VkDeviceContext()
    {

    }

    void VkDeviceContext::InitDeviceAndQueues(const vk::raii::Instance& instance, const VkDeviceContextDesc& desc)
    {
        vk::raii::PhysicalDevices availableDevices(instance);
        vk::raii::PhysicalDevice* chosenPhysicalDevice = nullptr;

        for (auto& physicalDevice : availableDevices)
        {
            LogDeviceProperties(physicalDevice);

            if (IsDeviceSuitableForDisplay(physicalDevice))
            {
                chosenPhysicalDevice = &physicalDevice;
                break;
            }
        }

        if (!chosenPhysicalDevice)
        {
            DEBUGGER_TRACE("no suitable vulkan device found");
            throw vk::SystemError(vk::Result::eErrorUnknown);
        }

        auto queueFamilies = QueryAvailableQueueFamilies(*chosenPhysicalDevice, desc.presentation_surface);

        vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
        vk::DeviceCreateInfo deviceInfo{
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = 3,
            .pQueueCreateInfos = queueFamilies.create_info.data(),
            .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
            .ppEnabledLayerNames = desc.required_layers.data(),
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
            .pEnabledFeatures = &deviceFeatures
        };

        _device = std::make_shared<vk::raii::Device>(*chosenPhysicalDevice, deviceInfo);

        _graphicsQueue = std::make_shared<vk::raii::Queue>(*_device, queueFamilies.direct_graphics_family_index, queueFamilies.direct_graphics_create_index);
        _computeQueue = std::make_shared<vk::raii::Queue>(*_device, queueFamilies.compute_family_index, queueFamilies.compute_create_index);
        _copyQueue = std::make_shared<vk::raii::Queue>(*_device, queueFamilies.copy_family_index, queueFamilies.copy_create_index);
        _presentQueue = std::make_shared<vk::raii::Queue>(*_device, queueFamilies.present_family_index, queueFamilies.present_create_index);

    }



}