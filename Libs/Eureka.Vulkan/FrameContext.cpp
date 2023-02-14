#include "FrameContext.hpp"

namespace eureka::vulkan
{



    FrameCommands::FrameCommands(std::shared_ptr<Device> device, Queue queue, FrameCommandsConfig config) :
        _config(config),
        _device(device),
        _pool(device, queue.Family())
    {

        _availableCommandBuffers.reserve(config.max_command_buffers);
        _usedCommandBuffers.reserve(config.max_command_buffers);
        _availableDoneFences.reserve(config.max_command_buffers);
        _usedDoneFences.reserve(config.max_command_buffers);
        _availableDoneTimelineSemaphore.reserve(config.max_command_buffers);
        _usedDoneTimelineSemaphore.reserve(config.max_command_buffers);
        _availableDoneBinarySemaphore.reserve(config.max_command_buffers);
        _usedDoneBinarySemaphore.reserve(config.max_command_buffers);
        _usedDoneFencesHandles.reserve(config.max_command_buffers);
        //_frameCommandsDoneFence = _device->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        //_frameDoneSemaphore = _device->createSemaphore(vk::SemaphoreCreateInfo{});

        for (auto i = 0u; i < _config.preallocated_command_buffers; ++i)
        {
            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            _availableDoneTimelineSemaphore.emplace_back(_device);
            _availableDoneBinarySemaphore.emplace_back(_device);
            _availableDoneFences.emplace_back(_device);   
        }

        //_device->resetFences(vk::ArrayProxy(_availableDoneFences.size(), _availableDoneFences.data()));

        _totalCommandBuffers = _config.preallocated_command_buffers;
    }

    VkFence FrameCommands::NewSubmitFence()
    {
        if (_availableDoneFences.empty())
        {
            if (_usedDoneFences.size() > _usedCommandBuffers.size()) throw std::logic_error("fence per submit");
            _availableDoneFences.emplace_back(_device);
        }

        auto newFence = std::move(_availableDoneFences.back());
        auto newFenceHandle = newFence.Get();
        _availableDoneFences.pop_back();

        _usedDoneFencesHandles.emplace_back(newFenceHandle);
        _usedDoneFences.emplace_back(std::move(newFence));
    
        return newFenceHandle;
    }

    //vk::Semaphore FrameCommands::NewDoneSemaphore() const
    //{
    //    return *_frameDoneSemaphore;
    //}

    SubmitCommandBuffer FrameCommands::NewSubmitCommandBuffer()
    {
        if (_availableCommandBuffers.empty())
        {
            if (_totalCommandBuffers >= _config.max_command_buffers) throw std::logic_error("too many command buffers per frame");

            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            ++_totalCommandBuffers;
        }

        if (_availableDoneTimelineSemaphore.empty())
        {
            _availableDoneTimelineSemaphore.emplace_back(_device);
        }

        _usedCommandBuffers.emplace_back(std::move(_availableCommandBuffers.back()));
        _usedDoneTimelineSemaphore.emplace_back(std::move(_availableDoneTimelineSemaphore.back()));


        _availableCommandBuffers.pop_back();
        _availableDoneTimelineSemaphore.pop_back();

        auto& timelineSemaphore = _usedDoneTimelineSemaphore.back();

        return SubmitCommandBuffer{ _usedCommandBuffers.back(), timelineSemaphore };
    }

    PresentCommandBuffer FrameCommands::NewPresentCommandBuffer()
    {
        if (_availableCommandBuffers.empty())
        {
            if (_totalCommandBuffers >= _config.max_command_buffers) throw std::logic_error("too many command buffers per frame");

            _availableCommandBuffers.emplace_back(_pool.AllocatePrimaryCommandBuffer());
            ++_totalCommandBuffers;
        }

        if (_availableDoneBinarySemaphore.empty())
        {
            _availableDoneBinarySemaphore.emplace_back(_device);
        }

        _usedCommandBuffers.emplace_back(std::move(_availableCommandBuffers.back()));
        _usedDoneBinarySemaphore.emplace_back(std::move(_availableDoneBinarySemaphore.back()));


        _availableCommandBuffers.pop_back();
        _availableDoneBinarySemaphore.pop_back();

        return PresentCommandBuffer{ _usedCommandBuffers.back() , _usedDoneBinarySemaphore.back() };
    }

    void FrameCommands::Reset()
    {
        if (!_usedCommandBuffers.empty())
        {
            _device->WaitForFences(_usedDoneFencesHandles);
            _device->ResetFences(_usedDoneFencesHandles);
            _pool.Reset();

            std::ranges::move(_usedCommandBuffers, std::back_inserter(_availableCommandBuffers));
            std::ranges::move(_usedDoneFences, std::back_inserter(_availableDoneFences));
            std::ranges::move(_usedDoneBinarySemaphore, std::back_inserter(_availableDoneBinarySemaphore));
            std::ranges::move(_usedDoneTimelineSemaphore, std::back_inserter(_availableDoneTimelineSemaphore));
            _usedCommandBuffers.clear();
            _usedDoneFences.clear();
            _usedDoneBinarySemaphore.clear();
            _usedDoneTimelineSemaphore.clear();
            _usedDoneFencesHandles.clear();
        }

    }

    FrameContext::FrameContext(
        std::shared_ptr<Device> device,
        Queue copyQueue,
        Queue graphicsQueue
    ) :
        _device(std::move(device)),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue)
    {
        //_maxFramesInFlight = _swapChain->ImageCount();


        // init commands
        for (auto i = 0u; i < _maxFramesInFlight; ++i)
        {
            _frameGraphicsCommands.emplace_back(_device, _graphicsQueue);
            _frameCopyCommands.emplace_back(_device, _copyQueue);
        }

        //bool found = false;
        //vk::Format depthFormat = DEFAULT_DEPTH_BUFFER_FORMAT;
        //for (auto format : { vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint })
        //{
        //    auto props = _deviceContext.PhysicalDevice()->getFormatProperties(format);

        //    if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        //    {
        //        depthFormat = format;
        //        found = true;
        //        break;
        //    }
        //}
        //DepthColorRenderPassConfig depthColorConfig
        //{
        //    .color_output_format = _swapChain->ImageFormat(),
        //    .depth_output_format = depthFormat
        //};

        //_renderPass = std::make_shared<DepthColorRenderPass>(_deviceContext, depthColorConfig);
        //_renderTargets = CreateDepthColorTargetForSwapChain(_deviceContext, *_swapChain, _renderPass);

        //_resizeConnection = _swapChain->ConnectResizeSlot(
        //    [this](uint32_t w, uint32_t h)
        //    {
        //        HandleSwapChainResize(w, h);
        //    }
        //);
    }


    FrameContext::~FrameContext()
    {

    }

    void FrameContext::SyncCurrentFrame()
    {
        _currentFrameGraphicsCommands = &_frameGraphicsCommands[_currentFrame];
        _currentFrameCopyCommands = &_frameCopyCommands[_currentFrame];
        _currentFrameGraphicsCommands->Reset();
        _currentFrameCopyCommands->Reset();
    }

    void FrameContext::BeginFrame()
    {
        _currentFrame = (_currentFrame + 1) % _maxFramesInFlight;
        SyncCurrentFrame();
    }

    void FrameContext::EndFrame()
    {

    }

    PresentCommandBuffer FrameContext::NewGraphicsPresentCommandBuffer()
    {
        return _currentFrameGraphicsCommands->NewPresentCommandBuffer();
    }

    SubmitCommandBuffer FrameContext::NewGraphicsCommandBuffer()
    {
        return _currentFrameGraphicsCommands->NewSubmitCommandBuffer();
    }

    SubmitCommandBuffer FrameContext::NewCopyCommandBuffer()
    {
        return _currentFrameCopyCommands->NewSubmitCommandBuffer();
    }

    VkFence FrameContext::NewGraphicsSubmitFence()
    {
        return _currentFrameGraphicsCommands->NewSubmitFence();
    }

    VkFence FrameContext::NewCopySubmitFence()
    {
        return _currentFrameCopyCommands->NewSubmitFence();
    }

}

