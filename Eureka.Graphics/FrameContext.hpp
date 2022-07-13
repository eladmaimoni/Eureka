#pragma once
#include "RenderTarget.hpp"

namespace eureka
{
    struct FrameRecording
    {
        //vk::CommandBuffer   command_buffer;
        vk::Semaphore       frame_available_wait_semaphore;
        vk::Semaphore       frame_done_signal_semaphore;
        vk::Fence           frame_done_signal_fence;
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
        std::vector<FrameCommands>                  _frameCommands;
        sigslot::signal<uint32_t, uint32_t>         _resizeSignal;
        sigslot::scoped_connection                  _resizeConnection;

        uint32_t                                    _maxFramesInFlight{};
        // current frame
        //uint32_t                                    _currentFrame;
        FrameCommands*                              _currentFrameCommnds{ nullptr };
        DepthColorRenderTarget*                     _currentRenderTarget{ nullptr };
        //vk::CommandBuffer                           _currentFrameCommandBuffer;
        //vk::Semaphore                               _currentFrameDoneSignalSemaphore;
        //vk::Fence                                   _currentFrameDoneSignalFence;
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

        FrameRecording BeginFrame()
        {
            auto [currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();
            // wait for current frame to finish execution before we reset its command pool (other frames can be in flight)
            _currentFrameCommnds = &_frameCommands[currentFrame];
            _currentRenderTarget = &_renderTargets[currentFrame];
            _currentFrameCommnds->Reset();
            //_currentFrame = currentFrame;

            //_currentFrameDoneSignalFence = currentFrameCommandRecord.DoneFence();
            //_currentFrameDoneSignalSemaphore = currentFrameCommandRecord.DoneSemaphore();
            //_currentFrameCommandBuffer = _currentFrameCommnds->NewCommandBuffer();

            //_currentFrameCommandBuffer.begin(vk::CommandBufferBeginInfo());

            return FrameRecording
            {
                .frame_available_wait_semaphore = imageReadySemaphore,
                .frame_done_signal_semaphore = _currentFrameCommnds->DoneSemaphore(),            
                .frame_done_signal_fence = _currentFrameCommnds->DoneFence()
            };
        }

        vk::CommandBuffer NewCommandBuffer()
        {
            return _currentFrameCommnds->NewCommandBuffer();
        }

        const vk::RenderPassBeginInfo& PrimaryRenderPassBeginInfo()
        {
            return _currentRenderTarget->BeginInfo();
        }

        //void BeginPrimaryRenderPass()
        //{
        //    _currentFrameCommandBuffer.beginRenderPass(_renderTargets[_currentFrame].BeginInfo(), vk::SubpassContents::eInline);
        //}

        //void EndPrimaryRenderPass()
        //{
        //    _currentFrameCommandBuffer.endRenderPass();
        //}

        //void EndFrameRecordingAndSubmit(
        //    dynamic_span<vk::Semaphore> waitList,
        //    dynamic_span<vk::PipelineStageFlags> waitStageMasks
        //)
        //{
        //    //_currentFrameCommandBuffer.end();

        //    vk::SubmitInfo submitInfo
        //    {
        //        .waitSemaphoreCount = static_cast<uint32_t>(waitList.size()),
        //        .pWaitSemaphores = waitList.data(),
        //        .pWaitDstStageMask = waitStageMasks.data(),
        //        .commandBufferCount = 1,
        //        .pCommandBuffers = &_currentFrameCommandBuffer,
        //        .signalSemaphoreCount = 1,
        //        .pSignalSemaphores = &_currentFrameDoneSignalSemaphore
        //    };

        //    _graphicsQueue->submit(
        //        { submitInfo },
        //        _currentFrameDoneSignalFence
        //    );
        //}

        void Present()
        {
            PROFILE_CATEGORIZED_SCOPE("Present", Profiling::Color::Gray, Profiling::PROFILING_CATEGORY_RENDERING);
            auto result = _swapChain->PresentLastAcquiredImageAsync(_currentFrameCommnds->DoneSemaphore());

            if (result != vk::Result::eSuccess)
            {
                DEBUGGER_TRACE("result = {}", result);
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

