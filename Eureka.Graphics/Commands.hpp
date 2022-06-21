#pragma once

#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"


namespace eureka
{
    class ScopedCommands
    {
        vkr::CommandBuffer& _commandBuffer;
    public:
        ScopedCommands(vkr::CommandBuffer& commandBuffer)
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


    class FrameCommands
    {
    private:
        CommandPool _pool;

        vkr::CommandBuffer _commandBuffer{ nullptr }; // could probably be allocated on demand
        vkr::Fence         _frameDoneFence{ nullptr };
        vkr::Semaphore     _frameDoneSemaphore{ nullptr };
        //
        // FrameCommands - 
        // the idea is to represent all command buffers that emanate from the same frame pool,
        // and the sync variables associated with synchronizing these commands with external
        // presentation engine.
        //
    public:
        FrameCommands(FrameCommands&& that) = default;
        FrameCommands& operator=(FrameCommands&& rhs) = default;
        FrameCommands() = default;
        FrameCommands(
            DeviceContext& deviceContext,
            Queue graphicsQueue
        ) : _pool(deviceContext.LogicalDevice(), CommandPoolDesc{.type = CommandPoolType::eLinear, .queue_family = graphicsQueue.Family() })
        {

            vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

            _frameDoneFence = deviceContext.LogicalDevice()->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
            _frameDoneSemaphore = deviceContext.LogicalDevice()->createSemaphore(vk::SemaphoreCreateInfo{});
            _commandBuffer = _pool.AllocatePrimaryCommandBuffer();

       
        }

        vk::Fence DoneFence() const 
        {
            return *_frameDoneFence;
        }

        vk::Semaphore DoneSemaphore() const
        {
            return *_frameDoneSemaphore;
        }
        vkr::CommandBuffer& CommandBuffer()
        {
            return _commandBuffer;
        }
        void Reset()
        {
            _pool.Reset();
        }
    };

    class CommandBuffer
    {
        CommandBuffer(std::shared_ptr<vkr::Queue> queue)
        {

        }
    };
}