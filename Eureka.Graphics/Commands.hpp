#pragma once

#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"


namespace eureka
{
    class ScopedCommands
    {
        vk::CommandBuffer _commandBuffer;
    public:
        EUREKA_NO_COPY_NO_MOVE(ScopedCommands)
        ScopedCommands(vkr::CommandBuffer& commandBuffer)
            : _commandBuffer(*commandBuffer)
        {
            _commandBuffer.begin(vk::CommandBufferBeginInfo());
        }
        ScopedCommands(vk::CommandBuffer commandBuffer)
            : _commandBuffer(commandBuffer)
        {
            _commandBuffer.begin(vk::CommandBufferBeginInfo());
        }
        ~ScopedCommands()
        {
            _commandBuffer.end();
        }
    };



    enum class CommandPoolType
    {
        eLinear = 0u,
        eTransientResettableBuffers = static_cast<uint32_t>(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
    };

    struct CommandPoolDesc
    {
        CommandPoolType type;
        uint32_t queue_family;
    };

    class CommandPool
    {    
        std::shared_ptr<vkr::Device> _device{nullptr};
        vkr::CommandPool _pool{nullptr};
    public:
        CommandPool() = default;
        ~CommandPool() = default;
        CommandPool(CommandPool&& that) = default;
        CommandPool& operator=(CommandPool&& rhs) = default;
        CommandPool(std::shared_ptr<vkr::Device> device, const CommandPoolDesc& desc)
            :
            _device(std::move(device)), 
            _pool(*_device, 
                vk::CommandPoolCreateInfo{
                    .flags = static_cast<vk::CommandPoolCreateFlagBits>(desc.type), 
                    .queueFamilyIndex = desc.queue_family }
            )
        {

        }

        void Reset()
        {
            _pool.reset();
        }
        vk::CommandPool Get() const { return *_pool; }
        vkr::CommandBuffer AllocatePrimaryCommandBuffer() const 
        {
            vk::CommandBufferAllocateInfo commandBufferAllocateInfo
            {
                .commandPool = *_pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
            };
            vk::CommandBuffer commandBuffer{ nullptr };
            auto device = **_device;

            VK_CHECK(vkAllocateCommandBuffers(device, &Vk(commandBufferAllocateInfo), &Vk(commandBuffer)));
            return vkr::CommandBuffer(*_device, commandBuffer, *_pool);
        }
    };



}