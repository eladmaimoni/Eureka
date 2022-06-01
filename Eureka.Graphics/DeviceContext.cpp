#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_mem_alloc.h"
#include "vk_error_handling.hpp"

namespace eureka
{

    bool IsDeviceSuitableForDisplay(const vkr::PhysicalDevice& device)
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



    struct DeviceCreationDesc
    {
        QueueFamilies                          queue_families;
        std::vector<std::vector<float>>        queue_priorities;
        std::vector<vk::DeviceQueueCreateInfo> create_info;
    };

    DeviceCreationDesc EnumerateQueueFamiliesForDeviceCreation(const vkr::PhysicalDevice& device, const DeviceContextConfig& desc)
    {
        // TODO TODO TODO better handling of prioirtes, presentation queues etc
        DeviceCreationDesc deviceCreationDesc;
        deviceCreationDesc.create_info.reserve(6);
        deviceCreationDesc.queue_priorities.reserve(6);

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
                deviceCreationDesc.queue_families.direct_graphics_family_index = i;
                deviceCreationDesc.queue_families.direct_graphics_family_max_count = desc.preferred_number_of_graphics_queues;

                graphicsCreateInfoPriority = &deviceCreationDesc.queue_priorities.emplace_back();
                graphicsCreateInfoPriority->resize(desc.preferred_number_of_graphics_queues, 1.0f);    
                graphicsCreateInfo = &deviceCreationDesc.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = deviceCreationDesc.queue_families.direct_graphics_family_index,
                        .queueCount = desc.preferred_number_of_graphics_queues,
                        .pQueuePriorities = graphicsCreateInfoPriority->data()
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

            if (i != deviceCreationDesc.queue_families.direct_graphics_family_index && queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute)
            {
                deviceCreationDesc.queue_families.compute_family_index = i;
                deviceCreationDesc.queue_families.compute_family_max_count = desc.preferred_number_of_compute_queues;
                computeCreateInfoPriority = &deviceCreationDesc.queue_priorities.emplace_back();
                computeCreateInfoPriority->resize(desc.preferred_number_of_compute_queues, 1.0f);

                computeCreateInfo = &deviceCreationDesc.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = deviceCreationDesc.queue_families.compute_family_index,
                        .queueCount = desc.preferred_number_of_compute_queues,
                        .pQueuePriorities = computeCreateInfoPriority->data()
                    }
                );
                break;
            }
            ++i;
        }

        if (!computeCreateInfo)
        {
            deviceCreationDesc.queue_families.compute_family_index = deviceCreationDesc.queue_families.direct_graphics_family_index;
            computeCreateInfo = graphicsCreateInfo;
            deviceCreationDesc.queue_families.direct_graphics_family_max_count += desc.preferred_number_of_compute_queues;
            graphicsCreateInfoPriority->insert(graphicsCreateInfoPriority->end(), desc.preferred_number_of_compute_queues, 1.0f);
            computeCreateInfoPriority = graphicsCreateInfoPriority;
            computeCreateInfo->setPQueuePriorities(computeCreateInfoPriority->data()); // reset pointer because insert might change it
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
                deviceCreationDesc.queue_families.copy_family_index = i;
                deviceCreationDesc.queue_families.copy_family_max_count = desc.preferred_number_of_copy_queues;
                copyCreateInfoPriority = &deviceCreationDesc.queue_priorities.emplace_back();
                copyCreateInfoPriority->resize(desc.preferred_number_of_copy_queues, 1.0f);

                copyCreateInfo = &deviceCreationDesc.create_info.emplace_back(
                    vk::DeviceQueueCreateInfo{
                        .flags = vk::DeviceQueueCreateFlags{},
                        .queueFamilyIndex = deviceCreationDesc.queue_families.copy_family_index,
                        .queueCount = desc.preferred_number_of_copy_queues,
                        .pQueuePriorities = copyCreateInfoPriority->data()
                    }
                );
                break;
            }
            ++i;
        }

        if (!copyCreateInfo && (queueFamilies[deviceCreationDesc.queue_families.compute_family_index].queueFlags & vk::QueueFlagBits::eTransfer))
        {
            deviceCreationDesc.queue_families.copy_family_index = deviceCreationDesc.queue_families.compute_family_index;
            copyCreateInfo = computeCreateInfo;

            deviceCreationDesc.queue_families.compute_family_max_count += desc.preferred_number_of_copy_queues;
            computeCreateInfoPriority->insert(computeCreateInfoPriority->end(), desc.preferred_number_of_copy_queues, 1.0f);

            copyCreateInfoPriority = computeCreateInfoPriority;
            copyCreateInfo->setPQueuePriorities(copyCreateInfoPriority->data()); // reset pointer because insert might change it
        }

        if (!copyCreateInfo)
        {
            deviceCreationDesc.queue_families.copy_family_index = deviceCreationDesc.queue_families.direct_graphics_family_index;
            computeCreateInfo = graphicsCreateInfo;

            deviceCreationDesc.queue_families.direct_graphics_family_max_count += desc.preferred_number_of_copy_queues;
            graphicsCreateInfoPriority->insert(graphicsCreateInfoPriority->end(), desc.preferred_number_of_copy_queues, 1.0f);

   
            copyCreateInfoPriority = graphicsCreateInfoPriority;
            copyCreateInfo->setPQueuePriorities(copyCreateInfoPriority->data()); // reset pointer because insert might change it
        }

        //
        // present queue
        //

        //if (desc.presentation_surface)
        //{
        //    if (device.getSurfaceSupportKHR(deviceCreationDesc.queue_families.copy_family_index, desc.presentation_surface))
        //    {
        //        deviceCreationDesc.queue_families.present_family_index = deviceCreationDesc.queue_families.copy_family_index;
        //    }
        //    else if (device.getSurfaceSupportKHR(deviceCreationDesc.queue_families.compute_family_index, desc.presentation_surface))
        //    {
        //        deviceCreationDesc.queue_families.present_family_index = deviceCreationDesc.queue_families.compute_family_index;

        //    }
        //    else if (device.getSurfaceSupportKHR(deviceCreationDesc.queue_families.direct_graphics_family_index, desc.presentation_surface))
        //    {
        //        deviceCreationDesc.queue_families.present_family_index = deviceCreationDesc.queue_families.direct_graphics_family_index;
        //    }
        //    else
        //    {
        //        bool found = false;
        //        for (i = 0u; i < queueFamilies.size(); ++i)
        //        {
        //            auto presentation = device.getSurfaceSupportKHR(i, desc.presentation_surface);

        //            if (presentation)
        //            {
        //                deviceCreationDesc.queue_families.present_family_index = i;
        //            
        //                auto priority = &deviceCreationDesc.queue_priorities.emplace_back().emplace_back(1.0f);
        //                deviceCreationDesc.create_info.emplace_back(
        //                    vk::DeviceQueueCreateInfo{
        //                        .flags = vk::DeviceQueueCreateFlags{},
        //                        .queueFamilyIndex = deviceCreationDesc.queue_families.present_family_index,
        //                        .queueCount = 1,
        //                        .pQueuePriorities = priority
        //                    }
        //                );
        //                break;
        //            }
        //        }

        //        if (!found)
        //        {
        //            // no present queue
        //            throw vk::SystemError(vk::Result::eErrorUnknown);
        //        }
        //    }

        //}

        
        return deviceCreationDesc;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                   DeviceContext 
    //
    //////////////////////////////////////////////////////////////////////////

    DeviceContext::DeviceContext(const Instance& instance, const DeviceContextConfig& desc)
    {
        Init(instance, desc);
    }

    void DeviceContext::Init(const Instance& instance, const DeviceContextConfig& desc)
    {
        assert(_device == nullptr);
        InitDeviceAndQueues(instance.Get(), desc);
        
        VmaAllocatorCreateInfo allocatorCreateInfo
        {
            .flags = 0,
            .physicalDevice = **_physicalDevice,
            .device = **_device,
            .instance = *instance.Get(),
            .vulkanApiVersion = instance.ApiVersion()
        };
        
        VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &_vmaAllocator));
    }

    DeviceContext::~DeviceContext()
    {
        vmaDestroyAllocator(_vmaAllocator);
    }

    void DeviceContext::InitializePresentationQueueFromExistingQueues(vk::SurfaceKHR presentationSurface)
    {
        assert(_device);
        assert(!_presentQueue);

        if (_physicalDevice->getSurfaceSupportKHR(_families.copy_family_index, presentationSurface))
        {
            _families.present_family_index = _families.copy_family_index;
            _presentQueue = _copyQueue.back();
        }
        else if (_physicalDevice->getSurfaceSupportKHR(_families.compute_family_index, presentationSurface))
        {
            _families.present_family_index = _families.compute_family_index;
            _presentQueue = _computeQueue.back();

        }
        else if (_physicalDevice->getSurfaceSupportKHR(_families.direct_graphics_family_index, presentationSurface))
        {
            _families.present_family_index = _families.direct_graphics_family_index;
            _presentQueue = _graphicsQueue.back();
        }
    }

    const std::shared_ptr<vkr::Queue>& DeviceContext::PresentQueue() const
    { 
        assert(_device); 
        if (!_presentQueue)
        {
            throw std::logic_error("present queue not created");
        }

        return _presentQueue;
    }

    void DeviceContext::InitDeviceAndQueues(const vkr::Instance& instance, const DeviceContextConfig& desc)
    {
        vkr::PhysicalDevices availableDevices(instance);
        vkr::PhysicalDevice* chosenPhysicalDevice = nullptr;

        for (auto& physicalDevice : availableDevices)
        {
            DEBUGGER_TRACE("Device name: {} type: {}", physicalDevice.getProperties().deviceName, physicalDevice.getProperties().deviceType);
         
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

        auto deviceCreationDesc = EnumerateQueueFamiliesForDeviceCreation(*chosenPhysicalDevice, desc);



        vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
        vk::DeviceCreateInfo deviceInfo{
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = 3,
            .pQueueCreateInfos = deviceCreationDesc.create_info.data(),
            .enabledLayerCount = static_cast<uint32_t>(desc.required_layers.size()),
            .ppEnabledLayerNames = desc.required_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(desc.required_extentions.size()),
            .ppEnabledExtensionNames = desc.required_extentions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        _device = std::make_shared<vkr::Device>(*chosenPhysicalDevice, deviceInfo);


        _physicalDevice = std::make_shared<vkr::PhysicalDevice>(std::move(*chosenPhysicalDevice));
    
        // TODO TODO TODO this will probably fail in other environments. families might be empty
        for (auto i = 0u; i < deviceCreationDesc.queue_families.direct_graphics_family_max_count; ++i)
        {
            _graphicsQueue.emplace_back(std::make_shared<vkr::Queue>(*_device, deviceCreationDesc.queue_families.direct_graphics_family_index, i));
        }
        for (auto i = 0u; i < deviceCreationDesc.queue_families.compute_family_max_count; ++i)
        {
            _computeQueue.emplace_back(std::make_shared<vkr::Queue>(*_device, deviceCreationDesc.queue_families.compute_family_index, i));
        }
        for (auto i = 0u; i < deviceCreationDesc.queue_families.copy_family_max_count; ++i)
        {
            _copyQueue.emplace_back(std::make_shared<vkr::Queue>(*_device, deviceCreationDesc.queue_families.copy_family_index, i));
        }

        _families = deviceCreationDesc.queue_families;
        
        if (desc.presentation_surface)
        {
            InitializePresentationQueueFromExistingQueues(desc.presentation_surface);
        }

    }


}