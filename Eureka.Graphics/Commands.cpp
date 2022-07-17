#include "Commands.hpp"

namespace eureka
{



    FrameCommands::FrameCommands(DeviceContext& deviceContext, Queue queue, FrameCommandsConfig config) :
        _config(config),
        _device(deviceContext.LogicalDevice()),
        _pool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eLinear, .queue_family = queue.Family() })
    {
        
        _availableCommandBuffers.reserve(config.max_command_buffers);
        _usedCommandBuffers.reserve(config.max_command_buffers);
        _availableDoneFences.reserve(config.max_command_buffers);
        _usedDoneFences.reserve(config.max_command_buffers);
        _availableDoneSemaphore.reserve(config.max_command_buffers);
        _usedDoneSemaphore.reserve(config.max_command_buffers);
        _usedDoneFencesHandles.reserve(config.max_command_buffers);
        //_frameCommandsDoneFence = _device->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        //_frameDoneSemaphore = _device->createSemaphore(vk::SemaphoreCreateInfo{});
        vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };
       
        for (auto i = 0u; i < _config.preallocated_command_buffers; ++i)
        {
            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            _availableDoneSemaphore.emplace_back(_device->createSemaphore(vk::SemaphoreCreateInfo{}));
            auto& fence = _availableDoneFences.emplace_back(_device->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }));
            _device->resetFences(*fence);
        }
        
        //_device->resetFences(vk::ArrayProxy(_availableDoneFences.size(), _availableDoneFences.data()));

        _totalCommandBuffers = _config.preallocated_command_buffers;
    }

    vk::Fence FrameCommands::NewSubmitFence()
    {
        if (_availableDoneFences.empty())
        {
            if (_usedDoneFences.size() > _usedCommandBuffers.size()) throw std::logic_error("fence per submit");

            auto& fence = _availableDoneFences.emplace_back(_device->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }));
            _device->resetFences(*fence);
        }
        _usedDoneFencesHandles.emplace_back(*_usedDoneFences.emplace_back(std::move(_availableDoneFences.back())));
        _availableDoneFences.pop_back();
        return *_usedDoneFences.back();
    }

    //vk::Semaphore FrameCommands::NewDoneSemaphore() const
    //{
    //    return *_frameDoneSemaphore;
    //}

    SubmitCommandBuffer FrameCommands::NewCommandBuffer()
    {
        if (_availableCommandBuffers.empty())
        {
            if (_totalCommandBuffers >= _config.max_command_buffers) throw std::logic_error("too many command buffers per frame");

            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            _availableDoneSemaphore.emplace_back(_device->createSemaphore(vk::SemaphoreCreateInfo{}));

            ++_totalCommandBuffers;
        }

        _usedCommandBuffers.emplace_back(std::move(_availableCommandBuffers.back()));
        _usedDoneSemaphore.emplace_back(std::move(_availableDoneSemaphore.back()));


        _availableCommandBuffers.pop_back();
        _availableDoneSemaphore.pop_back();

        return SubmitCommandBuffer{ *_usedCommandBuffers.back() , *_usedDoneSemaphore.back() };
    }

    void FrameCommands::Reset()
    {
        if (!_usedCommandBuffers.empty())
        {
            VK_CHECK(_device->waitForFences(_usedDoneFencesHandles, VK_TRUE, UINT64_MAX));
            _device->resetFences(_usedDoneFencesHandles);
            _pool.Reset();

            std::ranges::move(_usedCommandBuffers, std::back_inserter(_availableCommandBuffers));
            std::ranges::move(_usedDoneFences, std::back_inserter(_availableDoneFences));
            std::ranges::move(_usedDoneSemaphore, std::back_inserter(_availableDoneSemaphore));
            _usedCommandBuffers.clear();
            _usedDoneFences.clear();
            _usedDoneSemaphore.clear();
            _usedDoneFencesHandles.clear();
        }

    }

}