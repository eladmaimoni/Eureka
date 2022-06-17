#pragma once
#include "Commands.hpp"

namespace eureka
{
    struct FrameCommandsConfig
    {
        uint64_t max_command_buffers = 10;
        uint64_t preallocated_command_buffers = 5;
    };

    struct SubmitCommandBuffer
    {
        vk::CommandBuffer command_buffer;
        vk::Semaphore     done_semaphore;
    };

    class FrameCommands
    {
        /*
           FrameCommands -
           the idea is to represent
           - all command buffers that are associated with a {Frame, Queue, thread}
           - sync variables associated associated with all of these command buffers

           - when the last queue submit of each frame, the user should use fence and semaphore to signal all
           commands have been executed
           - prior to each frame, the user should wait on the Fence to ensure that the command buffers are no longer in use

        */
    private:
        FrameCommandsConfig _config;
        std::shared_ptr<vkr::Device> _device;
        CommandPool _pool;
        uint64_t _totalCommandBuffers{ 0 };
        std::vector<vkr::CommandBuffer> _availableCommandBuffers;
        std::vector<vkr::CommandBuffer> _usedCommandBuffers;
        std::vector<vkr::Fence> _availableDoneFences;
        std::vector<vkr::Fence> _usedDoneFences;
        std::vector<vk::Fence> _usedDoneFencesHandles;
        std::vector<vkr::Semaphore> _availableDoneSemaphore;
        std::vector<vkr::Semaphore> _usedDoneSemaphore;
    public:
        FrameCommands(FrameCommands&& that) = default;
        FrameCommands& operator=(FrameCommands&& rhs) = default;
        FrameCommands() = default;
        FrameCommands(
            DeviceContext& deviceContext,
            Queue queue,
            FrameCommandsConfig config = FrameCommandsConfig{}
        );

        vk::Fence NewSubmitFence();
        SubmitCommandBuffer NewCommandBuffer();

        void Reset();
    };



    class FrameContext
    {
    private:
        DeviceContext& _deviceContext;
        Queue                                       _copyQueue;
        Queue                                       _graphicsQueue;
        std::vector<FrameCommands>                  _frameGraphicsCommands;
        std::vector<FrameCommands>                  _frameCopyCommands;

        uint32_t                                    _maxFramesInFlight{ 2 };
        uint32_t                                    _currentFrame{ 1 };

        FrameCommands*                              _currentFrameGraphicsCommands{ nullptr };
        FrameCommands*                              _currentFrameCopyCommands{ nullptr };
    public:
        FrameContext(
            DeviceContext& deviceContext,
            Queue copyQueue,
            Queue graphicsQueue
        );

        void BeginFrame();

        void EndFrame()
        {

        }

        SubmitCommandBuffer NewGraphicsCommandBuffer()
        {
            return _currentFrameGraphicsCommands->NewCommandBuffer();
        }

        SubmitCommandBuffer NewCopyCommandBuffer()
        {
            return _currentFrameCopyCommands->NewCommandBuffer();
        }

        vk::Fence NewGraphicsSubmitFence()
        {
            return _currentFrameGraphicsCommands->NewSubmitFence();
        }

        vk::Fence NewCopySubmitFence()
        {
            return _currentFrameCopyCommands->NewSubmitFence();
        }
    };

}

