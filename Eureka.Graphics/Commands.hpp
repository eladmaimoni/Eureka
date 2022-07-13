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

    struct FrameCommandsConfig
    {
        uint64_t max_command_buffers = 10;
        uint64_t preallocated_command_buffers = 5;
    };

    class FrameCommands
    {
        /*
           FrameCommands -
           the idea is to represent all command buffers that emanate from the same frame pool each frame
           and the sync variables associated with synchronizing these commands with external
           presentation engine.

        */
    private:
        FrameCommandsConfig _config;
        CommandPool _pool;
        uint64_t _totalCommandBuffers{ 0 };
        svec15<vkr::CommandBuffer> _availableCommandBuffers;
        svec15<vkr::CommandBuffer> _usedCommandBuffers;


        vkr::Fence         _frameDoneFence{ nullptr };
        vkr::Semaphore     _frameDoneSemaphore{ nullptr };

    public:
        FrameCommands(FrameCommands&& that) = default;
        FrameCommands& operator=(FrameCommands&& rhs) = default;
        FrameCommands() = default;
        FrameCommands(
            DeviceContext& deviceContext,
            Queue queue,
            FrameCommandsConfig config = FrameCommandsConfig{}
        );

        vk::Fence DoneFence() const 
        {
            return *_frameDoneFence;
        }

        vk::Semaphore DoneSemaphore() const
        {
            return *_frameDoneSemaphore;
        }

        vk::CommandBuffer NewCommandBuffer();

        void Reset();
    };


}