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

    struct CommandPoolDesc
    {
        uint32_t queue_family;
    };

    class CommandPool
    {
        std::shared_ptr<vkr::Device> _device;
        vkr::CommandPool _pool{nullptr};

    public:
        CommandPool(std::shared_ptr<vkr::Device> device) 
            : _device(std::move(device))
        {

        };
        CommandPool(CommandPool&& that) = default;
        CommandPool& operator=(CommandPool&& rhs)
        {
            _device = std::move(rhs._device);
            _pool = std::move(rhs._pool);
            return *this;
        }
        CommandPool(
            std::shared_ptr<vkr::Device> device,
            const CommandPoolDesc& desc
            )
            : 
            _device(std::move(device)),
            _pool(
                *_device,
                vk::CommandPoolCreateInfo
                { 
                    .flags = {},  // no flags, means we can only reset the pool and not the command buffers
                    .queueFamilyIndex = desc.queue_family 
                }
                )
        {

        }

        void Reset()
        {
            _pool.reset();
        }

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

        vk::CommandPool Get() const {return *_pool;}
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
            DeviceContext& deviceContext
        ) : _pool(deviceContext.LogicalDevice(), CommandPoolDesc{.queue_family = deviceContext.Families().direct_graphics_family_index})
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