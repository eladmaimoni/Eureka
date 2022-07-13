#pragma once
#include "RenderTarget.hpp"

namespace eureka
{
    struct BeginFrameInfo
    {
        bool           frame_valid;
        vk::Semaphore  frame_available_wait_semaphore;
        vk::Semaphore  frame_done_signal_semaphore;
        vk::Fence      frame_done_graphics_signal_fence;
        vk::Fence      frame_done_copy_signal_fence;
    };

    class SwapChainFrameContext
    {
        /*
        this class represent a frame preparation
        -

        */
    private:
        DeviceContext& _deviceContext;
        Queue                                       _copyQueue;
        Queue                                       _graphicsQueue;
        std::shared_ptr<SwapChain>                  _swapChain;
        std::shared_ptr<DepthColorRenderPass>       _renderPass;
        std::vector<DepthColorRenderTarget>         _renderTargets;
        std::vector<FrameCommands>                  _frameGraphicsCommands;
        std::vector<FrameCommands>                  _frameCopyCommands;
        sigslot::signal<uint32_t, uint32_t>         _resizeSignal;
        sigslot::scoped_connection                  _resizeConnection;

        uint32_t                                    _maxFramesInFlight{};
        FrameCommands*                              _currentFrameGraphicsCommnds{ nullptr };
        FrameCommands*                              _currentFrameCopyCommnds{ nullptr };
        DepthColorRenderTarget*                     _currentRenderTarget{ nullptr };
    public:
        std::shared_ptr<DepthColorRenderPass> GetRenderPass() const
        {
            return _renderPass;
        }

        SwapChainFrameContext(
            DeviceContext& deviceContext,
            Queue copyQueue,
            Queue graphicsQueue,
            std::shared_ptr<SwapChain> swapChain
        );

        vk::Rect2D RenderArea() const
        {
            return _swapChain->RenderArea();
        }

        BeginFrameInfo BeginFrame()
        {
            auto [valid, currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();
            
            if (valid)
            {
                // wait for current frame to finish execution before we reset its command pool (other frames can be in flight)
                _currentFrameGraphicsCommnds = &_frameGraphicsCommands[currentFrame];
                _currentFrameCopyCommnds = &_frameCopyCommands[currentFrame];
                _currentRenderTarget = &_renderTargets[currentFrame];
                _currentFrameGraphicsCommnds->Reset();
                _currentFrameCopyCommnds->Reset();
                return BeginFrameInfo
                {
                    .frame_valid = true,
                    .frame_available_wait_semaphore = imageReadySemaphore,
                    .frame_done_signal_semaphore = _currentFrameGraphicsCommnds->DoneSemaphore(),
                    .frame_done_graphics_signal_fence = _currentFrameGraphicsCommnds->DoneFence(),
                    .frame_done_copy_signal_fence = _currentFrameCopyCommnds->DoneFence()
                };
            }
            else
            {
                return BeginFrameInfo
                {
                    .frame_valid = false
                };
            }


        }

        vk::CommandBuffer NewGraphicsCommandBuffer()
        {
            return _currentFrameGraphicsCommnds->NewCommandBuffer();
        }
        vk::CommandBuffer NewCopyCommandBuffer()
        {
            return _currentFrameCopyCommnds->NewCommandBuffer();
        }
        const vk::RenderPassBeginInfo& PrimaryRenderPassBeginInfo()
        {
            return _currentRenderTarget->BeginInfo();
        }

        void EndFrame()
        {
            PROFILE_CATEGORIZED_SCOPE("Present", Profiling::Color::Gray, Profiling::PROFILING_CATEGORY_RENDERING);
            auto result = _swapChain->PresentLastAcquiredImageAsync(_currentFrameGraphicsCommnds->DoneSemaphore());

            if (result == vk::Result::eErrorOutOfDateKHR)
            {
                _graphicsQueue->waitIdle();
                _renderTargets = CreateDepthColorTargetForSwapChain(
                    _deviceContext,
                    *_swapChain,
                    _renderPass
                );

                
            }

        }

        template <typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }
    private:

        void HandleSwapChainResize(uint32_t width, uint32_t height)
        {
            DEBUGGER_TRACE("handle swap chain resize");

            _graphicsQueue->waitIdle();
            _renderTargets = CreateDepthColorTargetForSwapChain(
                _deviceContext,
                *_swapChain,
                _renderPass
            );

            _resizeSignal(width, height);
        }
    };

}

