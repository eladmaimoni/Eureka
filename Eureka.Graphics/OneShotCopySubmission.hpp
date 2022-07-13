#pragma once
#include "DeviceContext.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "FrameContext.hpp"

namespace eureka
{
    struct OneShotCopySubmissionPacket
    {
        vk::CommandBuffer                  command_buffer{ nullptr };
        vkr::Semaphore                     done_timeline_semaphore{ nullptr };
        concurrencpp::result_promise<void> done_promise;
    };

    class OneShotSubmissionHandler
    {
        DeviceContext&                            _deviceContext;
        Queue                                     _copyQueue;
        Queue                                     _graphicsQueue;
        std::deque<OneShotCopySubmissionPacket>   _pendingOneShotCopies;

        // TODO linear ranges with fixed capacity and holes fixing
        std::vector<OneShotCopySubmissionPacket>  _executingOneShotCopies;
        std::vector<uint64_t>                     _executingOneShotSignalValues;
        std::vector<vk::Semaphore>                _executingOneShotSignalSemaphores;
        std::vector<vk::CommandBuffer>            _executingOneShotCommandBuffers;

        std::shared_ptr<SubmissionThreadExecutionContext> _submissionThreadExecutionContext;
        std::shared_ptr<SwapChainFrameContext> _frameContext;

    public:
        OneShotSubmissionHandler(DeviceContext& deviceContext, Queue copyQueue, Queue graphicsQueue, std::shared_ptr<SwapChainFrameContext> frameContext, std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext);
        future_t<void> AppendOneShotCopyCommandBufferSubmission(vk::CommandBuffer buffer);


        [[nodiscard]] auto ResumeOnRecordingContext()
        {
            return concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());
        }



        vk::CommandBuffer NewOneShotCopyCommandBuffer()
        {
            assert(tls_is_rendering_thread);
            return _frameContext->NewCopyCommandBuffer();
        }

        Queue& CopyQueue()
        {
            return _copyQueue;
        }

        Queue& GraphicsQueue()
        {
            return _graphicsQueue;
        }

        void SubmitPendingOneShotCopies(vk::Fence signalFence);

        void PollDoneOneShotSubmissions();
    };
}