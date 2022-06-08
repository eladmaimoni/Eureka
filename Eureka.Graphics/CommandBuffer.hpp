#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace eureka
{
    struct CommandPoolDesc
    {
        uint32_t queue_family;
    };

    class CommandPool
    {
        vkr::CommandPool _commandPool;
    public:
        CommandPool(const CommandPoolDesc& desc, const vkr::Device& device)
            : _commandPool(device, vk::CommandPoolCreateInfo{ .flags = {},.queueFamilyIndex = desc.queue_family })
        {

        }
    };

    struct CommandBufferDesc
    {
        
    };

    class CommandBuffer
    {
        CommandBuffer(std::shared_ptr<vkr::Queue> queue)
        {

        }
    };
}