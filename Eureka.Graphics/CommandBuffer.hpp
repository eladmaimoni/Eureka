#pragma once


namespace eureka
{
    struct CommandPoolDesc
    {
        uint32_t queue_family;
    };

    class CommandPool
    {
        vk::raii::CommandPool _commandPool;
    public:
        CommandPool(const CommandPoolDesc& desc, const vk::raii::Device& device)
            : _commandPool(device, vk::CommandPoolCreateInfo{ .flags = {},.queueFamilyIndex = desc.queue_family })
        {

        }
    };

    struct CommandBufferDesc
    {
        
    };

    class CommandBuffer
    {
        CommandBuffer(std::shared_ptr<vk::raii::Queue> queue)
        {

        }
    };
}