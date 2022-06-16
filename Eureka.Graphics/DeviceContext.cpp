#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_mem_alloc.h"
#include "vk_error_handling.hpp"
#include <unordered_map>

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


    enum class QueueType
    {
        Graphics,
        Compute,
        Copy
    };
    
    struct DeviceCreationDesc
    {
        std::vector<float>                     queue_priorities;
        std::vector<vk::DeviceQueueCreateInfo> create_info;
        std::unordered_map<uint32_t, uint32_t> queues_per_index;

        uint32_t                               graphics_idx;
        uint32_t                               compute_idx;
        uint32_t                               copy_idx;
    };

    struct IndexFlagsPair
    {
        uint32_t index;
        vk::QueueFlags flags;
    };

    DeviceCreationDesc EnumerateQueueFamiliesForDeviceCreation(const vkr::PhysicalDevice& device, const DeviceContextConfig& desc)
    {

        DeviceCreationDesc deviceCreationDesc;
        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
     
        DEBUGGER_TRACE("starting going through queues");
        
        std::unordered_map<QueueType, std::vector<IndexFlagsPair>> indicesOrderedBySpecificity;
        
        for (auto i = 0u; i < queueFamilies.size(); ++i)
        {
            const auto& props = queueFamilies[i];
          
            auto has_copy_flag = static_cast<bool>(props.queueFlags & vk::QueueFlagBits::eTransfer);
            auto has_compute_flag = static_cast<bool>(props.queueFlags & vk::QueueFlagBits::eCompute);
            auto has_graphics_flag = static_cast<bool>(props.queueFlags & vk::QueueFlagBits::eGraphics);
            
            if (has_graphics_flag)
            {
                auto& graphicsIndices = indicesOrderedBySpecificity[QueueType::Graphics];

                graphicsIndices.emplace_back(IndexFlagsPair{ .index = i, .flags = props.queueFlags });

                std::sort(graphicsIndices.begin(), graphicsIndices.end(), [](const IndexFlagsPair& a, const IndexFlagsPair& b)
                    {
                        int a_has_copy_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eTransfer);
                        int a_has_compute_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eCompute);
                        int b_has_copy_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eTransfer);
                        int b_has_compute_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eCompute);                      
                        return (a_has_copy_flag + a_has_compute_flag) < (b_has_copy_flag + b_has_compute_flag);
                    }
                );
            }

            if (has_compute_flag)
            {
                auto& computeIndices = indicesOrderedBySpecificity[QueueType::Compute];

                computeIndices.emplace_back(IndexFlagsPair{ .index = i, .flags = props.queueFlags });

                std::sort(computeIndices.begin(), computeIndices.end(), [](const IndexFlagsPair& a, const IndexFlagsPair& b)
                    {
                        int a_has_copy_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eTransfer);
                        int a_has_graphics_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eGraphics);
                        int b_has_copy_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eTransfer);
                        int b_has_graphics_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eGraphics);
                        return (a_has_copy_flag + a_has_graphics_flag) < (b_has_copy_flag + b_has_graphics_flag);
                    }
                );
            }

            if (has_copy_flag)
            {
                auto& copyIndices = indicesOrderedBySpecificity[QueueType::Copy];

                copyIndices.emplace_back(IndexFlagsPair{ .index = i, .flags = props.queueFlags });

                std::sort(copyIndices.begin(), copyIndices.end(), [](const IndexFlagsPair& a, const IndexFlagsPair& b)
                    {
                        int a_has_graphics_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eGraphics);
                        int a_has_compute_flag = static_cast<bool>(a.flags & vk::QueueFlagBits::eCompute);
                        int b_has_copy_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eTransfer);
                        int b_has_graphics_flag = static_cast<bool>(b.flags & vk::QueueFlagBits::eGraphics);
                        return (a_has_graphics_flag + a_has_compute_flag) < (b_has_copy_flag + b_has_graphics_flag);
                    }
                );
            }

        }

        auto& queuesPerIndex = deviceCreationDesc.queues_per_index;
        
        auto bestGraphicsIndex = indicesOrderedBySpecificity.at(QueueType::Graphics).at(0).index;
        auto bestComputeIndex = indicesOrderedBySpecificity.at(QueueType::Compute).at(0).index;
        auto bestCopyIndex = indicesOrderedBySpecificity.at(QueueType::Copy).at(0).index;

        queuesPerIndex[bestGraphicsIndex] = 0;
        queuesPerIndex[bestComputeIndex] = 0;
        queuesPerIndex[bestCopyIndex] = 0;
        
        

        //deviceCreationDesc.graphics_start = 0;
        queuesPerIndex[bestGraphicsIndex] += desc.preferred_number_of_graphics_queues;
        queuesPerIndex[bestComputeIndex] += desc.preferred_number_of_compute_queues;
        queuesPerIndex[bestCopyIndex] += desc.preferred_number_of_copy_queues;

        queuesPerIndex[bestGraphicsIndex] = std::min(queueFamilies[bestGraphicsIndex].queueCount, queuesPerIndex[bestGraphicsIndex]);
        queuesPerIndex[bestComputeIndex] = std::min(queueFamilies[bestComputeIndex].queueCount, queuesPerIndex[bestComputeIndex]);
        queuesPerIndex[bestCopyIndex] = std::min(queueFamilies[bestCopyIndex].queueCount, queuesPerIndex[bestCopyIndex]);

        deviceCreationDesc.graphics_idx = bestGraphicsIndex;
        deviceCreationDesc.compute_idx = bestComputeIndex;
        deviceCreationDesc.copy_idx = bestCopyIndex;

        //deviceCreationDesc[QueueType::Graphics] = QueueTypeInfo{ .family = bestGraphicsIndex, .max_count = queuesPerIndex[bestGraphicsIndex], .is_family_unique = (bestGraphicsIndex != bestComputeIndex && bestGraphicsIndex != bestCopyIndex) };
        //deviceCreationDesc[QueueType::Compute] = QueueTypeInfo{ .family = bestComputeIndex, .max_count = queuesPerIndex[bestComputeIndex], .is_family_unique = (bestComputeIndex != bestGraphicsIndex && bestComputeIndex != bestCopyIndex) };
        //deviceCreationDesc[QueueType::Copy] = QueueTypeInfo{ .family = bestCopyIndex, .max_count = queuesPerIndex[bestCopyIndex], .is_family_unique = (bestCopyIndex != bestGraphicsIndex && bestCopyIndex != bestComputeIndex) };

        auto totalQueues = 0;
        for (auto [idx, count] : queuesPerIndex)
        {
            totalQueues += count;
        }

        deviceCreationDesc.queue_priorities = std::vector<float>(totalQueues, 1.0f);

        std::vector<vk::DeviceQueueCreateInfo> createInfos;
        auto priorityPtr = deviceCreationDesc.queue_priorities.data();
        for (auto [idx, count] : queuesPerIndex)
        {         
            deviceCreationDesc.create_info.emplace_back(
                vk::DeviceQueueCreateInfo
                {
                   .flags = vk::DeviceQueueCreateFlags{},
                   .queueFamilyIndex = idx,
                   .queueCount = count,
                   .pQueuePriorities = priorityPtr
                }
            );

            priorityPtr += count;
        }

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

        _shaderCache = ShaderCache(_device);
    }

    DeviceContext::~DeviceContext()
    {
        if (_vmaAllocator)
        {
            vmaDestroyAllocator(_vmaAllocator);
        }
    }

    Queue DeviceContext::CreateGraphicsQueue()
    {
        return *TryCreateQueue(_preferredGraphicsIdx);
    }

    Queue DeviceContext::CreateComputeQueue()
    {
        return *TryCreateQueue(_preferredComputeIdx);
    }

    Queue DeviceContext::CreateCopyQueue()
    {
        return *TryCreateQueue(_preferredCopyIdx);
    }

    Queue DeviceContext::CreatePresentQueue(vk::SurfaceKHR presentationSurface)
    {
        std::optional<Queue> queue{ std::nullopt };
        if (_physicalDevice->getSurfaceSupportKHR(_preferredCopyIdx, presentationSurface))
        {
            queue = TryCreateQueue(_preferredCopyIdx);
        }
        if (!queue && _physicalDevice->getSurfaceSupportKHR(_preferredComputeIdx, presentationSurface))
        {
            queue = TryCreateQueue(_preferredComputeIdx);
        }
        if (!queue && _physicalDevice->getSurfaceSupportKHR(_preferredGraphicsIdx, presentationSurface))
        {
            queue = TryCreateQueue(_preferredGraphicsIdx);
        }
        return *queue;
    }

    //vk::Queue DeviceContext::PresentQueue() const
    //{ 
    //    assert(_device); 
    //    if (!_presentQueue)
    //    {
    //        throw std::logic_error("present queue not created");
    //    }

    //    return _presentQueue;
    //}

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
    
        _preferredGraphicsIdx = deviceCreationDesc.graphics_idx;
        _preferredComputeIdx = deviceCreationDesc.compute_idx;
        _preferredCopyIdx = deviceCreationDesc.copy_idx;
        for (auto [idx, count] : deviceCreationDesc.queues_per_index)
        {
            auto& queuesVec = _queuesByFamily[idx];
            for (auto i = 0u; i < count; ++i)
            {
                Queue q(*_device->getQueue(idx, i), idx);

                queuesVec.emplace_back(QueueManagement{q, false});
            }
        }
   
        //if (desc.presentation_surface)
        //{
        //    InitializePresentationQueueFromExistingQueues(desc.presentation_surface);
        //}

    }

    std::optional<Queue> eureka::DeviceContext::TryCreateQueue(uint32_t family)
    {
        for (auto& q : _queuesByFamily.at(family))
        {
            if (!q.is_taken)
            {
                q.is_taken = true;
                return q.queue;
            }
        }
        return std::nullopt;
    }


}