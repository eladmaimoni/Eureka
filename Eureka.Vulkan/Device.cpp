#include "Device.hpp"
#include "Result.hpp"
#include <assert.hpp>
#include <debugger_trace.hpp>
#include <macros.hpp>


namespace eureka::vulkan
{

    //
    // 
    //
    struct IndexFlagsPair
    {
        uint32_t index;
        VkQueueFlags flags;
    };

    bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, const DeviceConfig& config)
    {
        uint32_t propertyCount = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr));
        std::vector<VkExtensionProperties> extentionProperties(propertyCount);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extentionProperties.data()));

        for (auto availableExtention : extentionProperties)
        {
            std::string_view availableExtentionName(availableExtention.extensionName);
        
            DEBUGGER_TRACE("available device extention = {}", availableExtentionName);
        }


        for (const auto& requestedExtension : config.required_extentions)
        {
            bool found = false;
            for (const auto& availableExtention : extentionProperties)
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

        propertyCount = 0;
        VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, nullptr));
        std::vector<VkLayerProperties> layerProperties(propertyCount);
        VK_CHECK(vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, layerProperties.data()));

        for (const auto& availableLayer : layerProperties)
        {
            std::string_view availableLayerName(availableLayer.layerName);

            DEBUGGER_TRACE("available layer = {}", availableLayerName);
        }
        for (const auto& requestedLayer : config.required_layers)
        {
            bool found = false;
            for (const auto& availableLayer : layerProperties)
            {
                std::string_view availableLayerName(availableLayer.layerName);
                if (availableLayerName == requestedLayer)
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

    std::string to_string(VkPhysicalDeviceType deviceType)
    {
        std::string str;

        switch (deviceType)
        {
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_OTHER:
            str = "Other";
            break;
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            str = "Integrated GPU";
            break;
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            str = "Discrete GPU";
            break;
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            str = "Virtual GPU";
            break;
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU:
            str = "CPU";
            break;
        }
        return str;
    }

    Device::Device(std::shared_ptr<Instance> instance, DeviceConfig config) 
        : 
        _config(config),
        _instance(std::move(instance))
    {

        std::tie(_physicalDevice, _prettyName) = ChoosePhysicalDevice(config);


        auto createDesc = MakeDeviceCreationDesc(config);


        VkPhysicalDeviceFeatures deviceFeatures{ };
        
        void* deviceCreateInfoNext = nullptr;
        VkPhysicalDeviceVulkan12Features features12{};
        features12.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.timelineSemaphore = true;
       
        auto version = _instance->ApiVersion();

        VkPhysicalDeviceVulkan13Features features13{};
        if (version.major >= 1 && version.minor >= 3)
        {
            features13.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            features13.synchronization2 = true;
            features13.pNext = &features12;
            deviceCreateInfoNext = &features13;
        }
        else
        {
            deviceCreateInfoNext = &features12;
        }

        VkDeviceCreateInfo deviceCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features12,
            .flags = VkDeviceCreateFlags{},
            .queueCreateInfoCount = static_cast<uint32_t>(createDesc.queu_create_info.size()),
            .pQueueCreateInfos = createDesc.queu_create_info.data(),
            .enabledLayerCount = static_cast<uint32_t>(config.required_layers.size()),
            .ppEnabledLayerNames = config.required_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(config.required_extentions.size()),
            .ppEnabledExtensionNames = config.required_extentions.data(),
            .pEnabledFeatures = &deviceFeatures
        };


        VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice));
    
        volkLoadDevice(_logicalDevice);
        
        
        _preferredGraphicsFamily = createDesc.graphics_family;
        _preferredComputeFamily = createDesc.compute_family;
        _preferredCopyFamily = createDesc.copy_family;
        _preferredPresentationFamily = createDesc.presentation_family;
        _preferredGraphicsIndex = createDesc.graphics_index;
        _preferredComputeIndex = createDesc.compute_index;
        _preferredCopyIndex = createDesc.copy_index;
        _preferredPresentationIndex = createDesc.presentation_index;

        _queuesPerFamily = std::move(createDesc.queues_per_index);
  

    }

    std::tuple<VkPhysicalDevice, std::string> Device::ChoosePhysicalDevice(const DeviceConfig& config)
    {
        auto physicalDevices = _instance->EnumeratePhysicalDevices();

        VkPhysicalDevice chosenPhysicalDevice{};
        std::string deviceName;

        for (auto physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
            DEBUGGER_TRACE("Device name: {} type: {}", properties.deviceName, to_string(properties.deviceType));

            if (IsDeviceSuitable(physicalDevice, config))
            {
                chosenPhysicalDevice = physicalDevice;
                deviceName = properties.deviceName;
                break;
            }
        }

        if (!chosenPhysicalDevice)
        {
            DEBUGGER_TRACE("no suitable vulkan device found");
            throw ResultError(VkResult::VK_ERROR_EXTENSION_NOT_PRESENT, "no compatible device");
        }

        return { chosenPhysicalDevice, deviceName };
    }

    DeviceCreationDesc Device::MakeDeviceCreationDesc(const DeviceConfig& config)
    {
        DeviceCreationDesc deviceCreationDesc{};

        uint32_t propertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &propertyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamiltyProperties(propertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &propertyCount, queueFamiltyProperties.data());

        std::unordered_map<QueueType, std::vector<IndexFlagsPair>> indicesOrderedBySpecificity;

        for (auto i = 0u; i < queueFamiltyProperties.size(); ++i)
        {
            const auto& props = queueFamiltyProperties[i];

            auto has_copy_flag = static_cast<bool>(props.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
            auto has_compute_flag = static_cast<bool>(props.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
            auto has_graphics_flag = static_cast<bool>(props.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);


            if (has_graphics_flag)
            {
                auto& graphicsIndices = indicesOrderedBySpecificity[QueueType::Graphics];

                graphicsIndices.emplace_back(IndexFlagsPair{ .index = i, .flags = props.queueFlags });

                std::sort(graphicsIndices.begin(), graphicsIndices.end(), [](const IndexFlagsPair& a, const IndexFlagsPair& b)
                    {
                        int a_has_copy_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
                        int a_has_compute_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
                        int b_has_copy_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
                        int b_has_compute_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
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
                        int a_has_copy_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
                        int a_has_graphics_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
                        int b_has_copy_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
                        int b_has_graphics_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
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
                        int a_has_graphics_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
                        int a_has_compute_flag = static_cast<bool>(a.flags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
                        int b_has_copy_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
                        int b_has_graphics_flag = static_cast<bool>(b.flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
                        return (a_has_graphics_flag + a_has_compute_flag) < (b_has_copy_flag + b_has_graphics_flag);
                    }
                );
            }

        }

        auto& queuesPerFamily = deviceCreationDesc.queues_per_index;

        auto bestGraphicsFamily = indicesOrderedBySpecificity.at(QueueType::Graphics).at(0).index;
        auto bestComputeFamily = indicesOrderedBySpecificity.at(QueueType::Compute).at(0).index;
        auto bestCopyFamily = indicesOrderedBySpecificity.at(QueueType::Copy).at(0).index;
        auto bestPresentationFamily = INVALID_IDX;

        queuesPerFamily[bestGraphicsFamily] = 0;
        queuesPerFamily[bestComputeFamily] = 0;
        queuesPerFamily[bestCopyFamily] = 0;
        queuesPerFamily[bestGraphicsFamily] += config.preferred_number_of_graphics_queues;
        queuesPerFamily[bestComputeFamily] += config.preferred_number_of_compute_queues;
        queuesPerFamily[bestCopyFamily] += config.preferred_number_of_copy_queues;

        if (config.presentation_surface)
        {
            // we have a presentation surface.
            // if one of the families support presentation, we make it the preferred 
            if (FamiliySupportsPresentation(bestCopyFamily, config.presentation_surface))
            {
                queuesPerFamily[bestCopyFamily] += 1;
                bestPresentationFamily = bestCopyFamily;
            }
            else if (FamiliySupportsPresentation(bestComputeFamily, config.presentation_surface))
            {
                queuesPerFamily[bestComputeFamily] += 1;
                bestPresentationFamily = bestComputeFamily;
            }
            else if (FamiliySupportsPresentation(bestGraphicsFamily, config.presentation_surface))
            {
                queuesPerFamily[bestGraphicsFamily] += 1;
                bestPresentationFamily = bestGraphicsFamily;
            }

        }

        queuesPerFamily[bestGraphicsFamily] = std::min(queueFamiltyProperties[bestGraphicsFamily].queueCount, queuesPerFamily[bestGraphicsFamily]);
        queuesPerFamily[bestComputeFamily] = std::min(queueFamiltyProperties[bestComputeFamily].queueCount, queuesPerFamily[bestComputeFamily]);
        queuesPerFamily[bestCopyFamily] = std::min(queueFamiltyProperties[bestCopyFamily].queueCount, queuesPerFamily[bestCopyFamily]);

        deviceCreationDesc.graphics_family = bestGraphicsFamily;
        deviceCreationDesc.compute_family = bestComputeFamily;
        deviceCreationDesc.copy_family = bestCopyFamily;
        deviceCreationDesc.presentation_family = bestPresentationFamily;

        
        auto totalQueues = 0;
        for (auto [idx, count] : queuesPerFamily)
        {
            totalQueues += count;
        }

        //
        // determine which index each family will use
        // by default, they all use the 0 index
        //
        std::unordered_map<uint32_t, uint32_t> usedPerFamily;
        deviceCreationDesc.graphics_index = 0;
        deviceCreationDesc.compute_index = 0;
        deviceCreationDesc.copy_index = 0;
        deviceCreationDesc.presentation_index = INVALID_IDX;

        usedPerFamily[deviceCreationDesc.graphics_family] = 1;
        usedPerFamily[deviceCreationDesc.compute_family] = 1;
        usedPerFamily[deviceCreationDesc.copy_family] = 1;
   

        //
        // determine index
        //
        if (deviceCreationDesc.compute_family == deviceCreationDesc.graphics_family && usedPerFamily[deviceCreationDesc.graphics_family] < queuesPerFamily[deviceCreationDesc.graphics_family])
        {
            deviceCreationDesc.compute_index = usedPerFamily[deviceCreationDesc.graphics_family]++;
        }
        else if (deviceCreationDesc.compute_family == deviceCreationDesc.copy_family && usedPerFamily[deviceCreationDesc.copy_family] < queuesPerFamily[deviceCreationDesc.copy_family])
        {
            deviceCreationDesc.compute_index = usedPerFamily[deviceCreationDesc.copy_family]++;
        }

        if (deviceCreationDesc.copy_family == deviceCreationDesc.graphics_family && usedPerFamily[deviceCreationDesc.graphics_family] < queuesPerFamily[deviceCreationDesc.graphics_family])
        {
            deviceCreationDesc.copy_family = usedPerFamily[deviceCreationDesc.graphics_family]++;
        }

        if (config.presentation_surface)
        {
            deviceCreationDesc.presentation_index = 0;

            if (deviceCreationDesc.presentation_family == deviceCreationDesc.copy_family && usedPerFamily[deviceCreationDesc.copy_family] < queuesPerFamily[deviceCreationDesc.copy_family])
            {
                deviceCreationDesc.presentation_index = usedPerFamily[deviceCreationDesc.copy_family]++;
            }
            else if (deviceCreationDesc.presentation_family == deviceCreationDesc.compute_family && usedPerFamily[deviceCreationDesc.compute_family] < queuesPerFamily[deviceCreationDesc.compute_family])
            {
                deviceCreationDesc.presentation_index = usedPerFamily[deviceCreationDesc.compute_family]++;
            }
            else if (deviceCreationDesc.presentation_family == deviceCreationDesc.graphics_family && usedPerFamily[deviceCreationDesc.graphics_family] < queuesPerFamily[deviceCreationDesc.graphics_family])
            {
                deviceCreationDesc.presentation_index = usedPerFamily[deviceCreationDesc.graphics_family]++;
            }
        }

        deviceCreationDesc.queue_priorities = std::vector<float>(totalQueues, 1.0f);

        std::vector<VkDeviceQueueCreateInfo> createInfos;
        auto priorityPtr = deviceCreationDesc.queue_priorities.data();
        for (auto [idx, count] : queuesPerFamily)
        {
            deviceCreationDesc.queu_create_info.emplace_back(
                VkDeviceQueueCreateInfo
                {
                   .sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                   .flags = VkDeviceQueueCreateFlags{},
                   .queueFamilyIndex = idx,
                   .queueCount = count,
                   .pQueuePriorities = priorityPtr
                }
            );

            priorityPtr += count;
        }

        return deviceCreationDesc;
    }

    Device::~Device()
    {
        if (_logicalDevice)
        {
            vkDestroyDevice(_logicalDevice, nullptr);
        }   
    }

    Queue Device::GetGraphicsQueue()
    {
        VkQueue queue{};
        vkGetDeviceQueue(_logicalDevice, _preferredGraphicsFamily, _preferredGraphicsIndex, &queue);
      
        return Queue(queue, _preferredGraphicsFamily);
    }

    Queue Device::GetComputeQueue()
    {
        VkQueue queue{};
        vkGetDeviceQueue(_logicalDevice, _preferredComputeFamily, _preferredComputeIndex, &queue);

        return Queue(queue, _preferredComputeFamily);
    }

    Queue Device::GetCopyQueue()
    {
        VkQueue queue{};
        vkGetDeviceQueue(_logicalDevice, _preferredCopyFamily, _preferredCopyIndex, &queue);

        return Queue(queue, _preferredCopyFamily);
    }

    Queue Device::GetPresentQueue()
    {
        if (_preferredPresentationFamily == INVALID_IDX || _preferredPresentationIndex == INVALID_IDX)
        {
            throw std::logic_error("no present queue");
        }
        
        VkQueue queue{};
        vkGetDeviceQueue(_logicalDevice, _preferredPresentationFamily, _preferredPresentationIndex, &queue);

        return Queue(queue, _preferredPresentationFamily);
    }



    bool Device::FamiliySupportsPresentation(uint32_t family, VkSurfaceKHR presentationSurface)
    {
        VkBool32 supported = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, family, presentationSurface, &supported));

        return static_cast<bool>(supported);
    }

    dspan<const char*> Device::EnabledExtentions() const
    {
        // that const cast is OK I guess
        auto ptr = const_cast<const char**>(_config.required_extentions.data());
        return dspan<const char*>(ptr, _config.required_extentions.size());
    }

    std::string_view Device::GetPrettyName() const
    {
        return _prettyName;
    }

    VkSampler Device::CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo) const
    {
        VkSampler result{};
        VK_CHECK(vkCreateSampler(_logicalDevice, &samplerCreateInfo, nullptr, &result));
        return result;
    }

    void Device::DestroySampler(VkSampler sampler) const
    {
        vkDestroySampler(_logicalDevice, sampler, nullptr);
    }

    VkImageView Device::CreateImageView(const VkImageViewCreateInfo& imageViewCreateInfo) const
    {
        VkImageView result{};
        VK_CHECK(vkCreateImageView(_logicalDevice, &imageViewCreateInfo, nullptr, &result));
        return result;
    }

    void Device::DestroyImageView(VkImageView view) const
    {
        vkDestroyImageView(_logicalDevice, view, nullptr);
    }

    VkSemaphore Device::CreateSemaphore(VkSemaphoreCreateInfo semaphoreCreateInfo) const
    {
        VkSemaphore result{};
        VK_CHECK(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &result));
        return result;
    }

    void Device::DestroySemaphore(VkSemaphore semaphore) const
    {
        vkDestroySemaphore(_logicalDevice, semaphore, nullptr);
    }

    uint64_t Device::GetSemaphoreCounterValue(VkSemaphore semaphore) const
    {
        uint64_t result{};
        VK_CHECK(vkGetSemaphoreCounterValue(_logicalDevice, semaphore, &result));
        return result;
    }

    void Device::SignalSemaphore(const VkSemaphoreSignalInfo& signalInfo) const
    {
        VK_CHECK(vkSignalSemaphore(_logicalDevice, &signalInfo));
    }

    VkResult Device::WaitSemaphores(const VkSemaphoreWaitInfo& waitInfo, uint64_t timeout) const
    {
        return vkWaitSemaphores(_logicalDevice, &waitInfo, timeout);
    }

    VkSurfaceCapabilitiesKHR Device::GetSwapchainCapabilities(VkSurfaceKHR surface) const
    {
        VkSurfaceCapabilitiesKHR result;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, surface, &result));
        return result;
    }

    SwapChainSupport Device::GetSwapChainSupport(VkSurfaceKHR surface) const
    {
        SwapChainSupport support{};
        uint32_t count = 0;

        support.capabilities = GetSwapchainCapabilities(surface);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &count, nullptr)); support.formats.resize(count);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &count, support.formats.data()));
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, surface, &count, nullptr)); support.present_modes.resize(count);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, surface, &count, support.present_modes.data()));

        return support;
    }

    VkSwapchainKHR Device::CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo) const
    {
        VkSwapchainKHR result{};
        VK_CHECK(vkCreateSwapchainKHR(_logicalDevice, &createInfo, nullptr, &result));
        return result;
    }

    std::vector<VkImage> Device::GetSwapchainImages(VkSwapchainKHR swapchain) const
    {
        uint32_t count = 0;
        std::vector<VkImage> result;
        VK_CHECK(vkGetSwapchainImagesKHR(_logicalDevice, swapchain, &count, nullptr));
        result.resize(count);
        VK_CHECK(vkGetSwapchainImagesKHR(_logicalDevice, swapchain, &count, result.data()));
        return result;
    }

    SwapChainImageAquisition Device::AcquireNextSwapChainImage(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore imageReady, VkFence fence) const
    {
        SwapChainImageAquisition result{};
        result.result = vkAcquireNextImageKHR(_logicalDevice, swapchain, timeout, imageReady, fence, &result.index);
        return result;
    }

    void Device::DestroySwapChain(VkSwapchainKHR swapchain) const
    {
        vkDestroySwapchainKHR(_logicalDevice, swapchain, nullptr);
    }

    VkDevice Device::GetDevice() const
    {
        return _logicalDevice;
    }

    VkPhysicalDevice Device::GetPhysicalDevice() const
    {
        return _physicalDevice;
    }

    VkPipelineCache Device::CreatePipelineCache() const
    {
        VkPipelineCache result{};

        VkPipelineCacheCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
        };

        VK_CHECK(vkCreatePipelineCache(_logicalDevice, &createInfo, nullptr, &result));
    
        return result;
    }

    void Device::DestroyPipelineCache(VkPipelineCache pipelineCache) const
    {
        vkDestroyPipelineCache(_logicalDevice, pipelineCache, nullptr);
    }

    VkShaderModule Device::CreateShaderModule(const VkShaderModuleCreateInfo& shaderModuleCreateInfo) const
    {
        VkShaderModule result{};

        VK_CHECK(vkCreateShaderModule(_logicalDevice, &shaderModuleCreateInfo, nullptr, &result));

        return result;
    }

    void Device::DestroyShaderModule(VkShaderModule shaderModule) const
    {
        vkDestroyShaderModule(_logicalDevice, shaderModule, nullptr);
    }

    void Device::DestroyCommandPool(VkCommandPool commandPool) const
    {
        vkDestroyCommandPool(_logicalDevice, commandPool, nullptr);
    }

    VkRenderPass Device::CreateRenderPass(const VkRenderPassCreateInfo& renderPassCreateInfo) const
    {
        VkRenderPass result{};

        VK_CHECK(vkCreateRenderPass(_logicalDevice, &renderPassCreateInfo, nullptr, &result));

        return result;
    }
    void Device::DestroyRenderPass(VkRenderPass renderPass) const
    {
        vkDestroyRenderPass(_logicalDevice, renderPass, nullptr);
    }

    VkFramebuffer Device::CreateFrameBuffer(const VkFramebufferCreateInfo& framebufferCreateInfo) const
    {
        VkFramebuffer result{};
        VK_CHECK(vkCreateFramebuffer(_logicalDevice, &framebufferCreateInfo, nullptr, &result));
        return result;
    }


    VkCommandPool Device::CreateCommandPool(const VkCommandPoolCreateInfo& commandPoolCreateInfo) const
    {
        VkCommandPool result{};
        VK_CHECK(vkCreateCommandPool(_logicalDevice, &commandPoolCreateInfo, nullptr, &result));
        return result;
    }

    void Device::ResetCommandPool(VkCommandPool commandPool) const
    {
        VK_CHECK(vkResetCommandPool(_logicalDevice, commandPool, 0));
    }

    VkCommandBuffer Device::AllocatePrimaryCommandBuffer(VkCommandPool commandPool) const
    {
        VkCommandBufferAllocateInfo allocationInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
  
        VkCommandBuffer result{};
        VK_CHECK(vkAllocateCommandBuffers(_logicalDevice, &allocationInfo, &result));
        return result;
    }

    void Device::DestroyFrameBuffer(VkFramebuffer frameBuffer) const
    {
        vkDestroyFramebuffer(_logicalDevice, frameBuffer, nullptr);
    }

    VkFormatProperties Device::GetFormatProperties(VkFormat format) const
    {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
        return props;
    }

    void Device::UpdateDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet) const
    {
        vkUpdateDescriptorSets(_logicalDevice, 1u, &writeDescriptorSet, 0u, nullptr);
    }

    void Device::UpdateDescriptorSet(dynamic_cspan<VkWriteDescriptorSet> writeDescriptorSet) const
    {
        auto count = static_cast<uint32_t>(writeDescriptorSet.size());
        vkUpdateDescriptorSets(_logicalDevice, count, writeDescriptorSet.data(), 0u, nullptr);
    }

    void Device::FreeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set) const
    {
        VK_CHECK(vkFreeDescriptorSets(_logicalDevice, pool, 1, &set));
    }

    VkDescriptorPool Device::CreateDescriptorPool(const VkDescriptorPoolCreateInfo& descriptorPoolCreateInfo) const
    {
        VkDescriptorPool result{};
        VK_CHECK(vkCreateDescriptorPool(_logicalDevice, &descriptorPoolCreateInfo, nullptr,&result));
        return result;
    }

    void Device::DestroyDescriptorPool(VkDescriptorPool pool) const
    {
        vkDestroyDescriptorPool(_logicalDevice, pool, nullptr);
    }

    VkDescriptorSet Device::AllocateDescriptorSet(const VkDescriptorSetAllocateInfo& allocInfo) const
    {
        assert(allocInfo.descriptorSetCount == 1);
        VkDescriptorSet result{};
        VK_CHECK(vkAllocateDescriptorSets(_logicalDevice, &allocInfo, &result));
        return result;
    }

    VkDescriptorSet Device::TryAllocateDescriptorSet(const VkDescriptorSetAllocateInfo& allocInfo) const
    {
        assert(allocInfo.descriptorSetCount == 1);
        VkDescriptorSet result{};
        auto vkResult = vkAllocateDescriptorSets(_logicalDevice, &allocInfo, &result);

        if (vkResult == VkResult::VK_SUCCESS)
        {
            return result;
        }
        else if (vkResult == VkResult::VK_ERROR_FRAGMENTED_POOL || vkResult == VkResult::VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            return nullptr;
        }
        else
        {
            throw ResultError(vkResult, "vulkan api error");
        }
    }

    VkFence Device::CreateFence(const VkFenceCreateInfo& fenceCreateInfo) const
    {
        VkFence result;
        VK_CHECK(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &result));
        return result;
    }

    void Device::DestroyFence(VkFence fence) const
    {
        vkDestroyFence(_logicalDevice, fence, nullptr);
    }

    void Device::WaitForFence(VkFence fence) const
    {
        VK_CHECK(vkWaitForFences(_logicalDevice, 1u, &fence, VK_TRUE, UINT64_MAX));
    }

    void Device::WaitForFences(dynamic_cspan<VkFence> fences) const
    {
        VK_CHECK(vkWaitForFences(_logicalDevice, static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
    }

    void Device::ResetFence(VkFence fence) const
    {
        VK_CHECK(vkResetFences(_logicalDevice, 1u, &fence));
    }

    void Device::ResetFences(dynamic_cspan<VkFence> fences) const
    {
        VK_CHECK(vkResetFences(_logicalDevice, static_cast<uint32_t>(fences.size()), fences.data()));
    }

    VkDescriptorSetLayout Device::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const
    {
        VkDescriptorSetLayout result;
        VK_CHECK(vkCreateDescriptorSetLayout(_logicalDevice, &info, nullptr, &result));
        return result;
    }

    void Device::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) const
    {
        vkDestroyDescriptorSetLayout(_logicalDevice, descriptorSetLayout, nullptr);
    }

    VkPipelineLayout Device::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& pipelineLayoutCreateInfo) const
    {
        VkPipelineLayout result;
        VK_CHECK(vkCreatePipelineLayout(_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &result));
        return result;
    }
    
    void Device::DestroyPipelineLayout(VkPipelineLayout pipelineLayout) const
    {
        vkDestroyPipelineLayout(_logicalDevice, pipelineLayout, nullptr);
    }

    VkPipeline Device::CreatePipeline(const VkGraphicsPipelineCreateInfo& pipelineCreateInfo, VkPipelineCache pipelineCache) const
    {
        VkPipeline result;
        VK_CHECK(vkCreateGraphicsPipelines(_logicalDevice, pipelineCache, 1u, &pipelineCreateInfo, nullptr, &result));
        return result;
    }

    void Device::DestroyPipeline(VkPipeline pipeline) const
    {
        vkDestroyPipeline(_logicalDevice, pipeline, nullptr);
    }

    std::shared_ptr<Device> MakeDefaultDevice(std::shared_ptr<Instance> instance, VkSurfaceKHR presentationSurface)
    {
        DeviceConfig deviceConfig{};

   
        deviceConfig.required_extentions.emplace_back(DEVICE_EXTENTION_SWAPCHAIN);
        deviceConfig.presentation_surface = presentationSurface;

        auto version = instance->ApiVersion();

        if (version.major == 1 && version.minor < 3)
        {        
            deviceConfig.required_extentions.emplace_back(DEVICE_EXTENTION_PRE13_SYNCHRONIZATION2);
            deviceConfig.required_extentions.emplace_back("VK_KHR_create_renderpass2");
            deviceConfig.required_extentions.emplace_back("VK_KHR_synchronization2");
            deviceConfig.required_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
            //deviceConfig.required_layers.emplace_back(DEVICE_LAYER_PRE13_SYNCHRONIZATION2);

            // I think synchronization2 is dependant on VK_KHR_get_physical_device_properties2 
            // https://vulkan.lunarg.com/doc/view/1.3.236.0/windows/1.3-extensions/vkspec.html#VK_KHR_synchronization2
            //deviceConfig.required_extentions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            //deviceConfig.required_extentions.emplace_back(DEVICE_EXTENTION_PRE13_SYNCHRONIZATION2);

        }

        DEBUG_ONLY(deviceConfig.required_layers.emplace_back(DEVICE_LAYER_VALIDATION));
        
        return std::make_shared<Device>(std::move(instance), std::move(deviceConfig));
    }

}

