#include "Commands.hpp"

namespace eureka
{


    FrameCommands::FrameCommands(DeviceContext& deviceContext, Queue queue, FrameCommandsConfig config) :
        _config(config),
        _device(deviceContext.LogicalDevice()),
        _pool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eLinear, .queue_family = queue.Family() })
    {
        vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

        _frameCommandsDoneFence = deviceContext.LogicalDevice()->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        _frameDoneSemaphore = deviceContext.LogicalDevice()->createSemaphore(vk::SemaphoreCreateInfo{});
        _device->resetFences({ *_frameCommandsDoneFence });
        for (auto i = 0u; i < _config.preallocated_command_buffers; ++i)
        {
            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
        }
        _totalCommandBuffers = _config.preallocated_command_buffers;
    }

    vk::CommandBuffer FrameCommands::NewCommandBuffer()
    {
        if (_availableCommandBuffers.empty())
        {
            if (_totalCommandBuffers >= _config.max_command_buffers) throw std::logic_error("too many command buffers per frame");
            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            ++_totalCommandBuffers;
        }

        _usedCommandBuffers.emplace_back(std::move(_availableCommandBuffers.back()));
        _availableCommandBuffers.pop_back();

        return *_usedCommandBuffers.back();
    }

    void FrameCommands::Reset()
    {
        if (!_usedCommandBuffers.empty())
        {
            VK_CHECK(_device->waitForFences({ *_frameCommandsDoneFence }, VK_TRUE, UINT64_MAX));
            _device->resetFences({ *_frameCommandsDoneFence });
            _pool.Reset();

            std::ranges::move(_usedCommandBuffers, std::back_inserter(_availableCommandBuffers));
            _usedCommandBuffers.clear();
        }

    }

}