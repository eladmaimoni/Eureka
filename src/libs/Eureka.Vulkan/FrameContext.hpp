#pragma once
#include "../Eureka.Vulkan/Commands.hpp"
#include "../Eureka.Vulkan/Synchronization.hpp"

namespace eureka::vulkan
{
    struct FrameCommandsConfig
    {
        uint64_t max_command_buffers = 10;
        uint64_t preallocated_command_buffers = 5;
    };

    struct SubmitCommandBuffer
    {
        LinearCommandBufferHandle command_buffer;
        CounterSemaphoreHandle    done_semaphore;
    };

    struct PresentCommandBuffer
    {
        LinearCommandBufferHandle      command_buffer;
        BinarySemaphoreHandle     done_semaphore;
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
        FrameCommandsConfig              _config;
        std::shared_ptr<Device>          _device;
        LinearCommandPool                _pool;
        uint64_t                         _totalCommandBuffers{ 0 };
        std::vector<LinearCommandBufferHandle> _availableCommandBuffers;
        std::vector<LinearCommandBufferHandle> _usedCommandBuffers;
        std::vector<Fence>               _availableDoneFences;
        std::vector<Fence>               _usedDoneFences;
        std::vector<VkFence>               _usedDoneFencesHandles;
        std::vector<BinarySemaphore>     _availableDoneBinarySemaphore;
        std::vector<BinarySemaphore>     _usedDoneBinarySemaphore;
        std::vector<CounterSemaphore>    _availableDoneTimelineSemaphore;
        std::vector<CounterSemaphore>    _usedDoneTimelineSemaphore;
    public:
        FrameCommands(FrameCommands&& that) = default;
        FrameCommands& operator=(FrameCommands&& rhs) = default;
        FrameCommands(
            std::shared_ptr<Device> device,
            Queue queue,
            FrameCommandsConfig config = FrameCommandsConfig{}
        );

        VkFence NewSubmitFence();
        SubmitCommandBuffer NewSubmitCommandBuffer();
        PresentCommandBuffer NewPresentCommandBuffer();

        void Reset();
    };



    class FrameContext
    {
    private:
        std::shared_ptr<Device>                     _device;
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
            std::shared_ptr<Device> device,
            Queue copyQueue,
            Queue graphicsQueue
        );
        ~FrameContext();
        void SyncCurrentFrame();
        void BeginFrame();
        void EndFrame();
        PresentCommandBuffer NewGraphicsPresentCommandBuffer();
        SubmitCommandBuffer NewGraphicsCommandBuffer();
        SubmitCommandBuffer NewCopyCommandBuffer();
        VkFence NewGraphicsSubmitFence();
        VkFence NewCopySubmitFence();
    };

}

