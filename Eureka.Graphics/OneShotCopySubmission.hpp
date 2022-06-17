#pragma once
#include "DeviceContext.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "FrameContext.hpp"

namespace eureka
{

    struct OneShotSubmissionPacket
    {
        vk::CommandBuffer                  command_buffer{ nullptr };
        vkr::Semaphore                     done_timeline_semaphore{ nullptr };
        concurrencpp::result_promise<void> done_promise;
    };

    struct ExecutingneShotSubmissions
    {
        ExecutingneShotSubmissions()
        {
            // TODO fixed capacity vector or array
            done_signal_values.reserve(100);
            done_signal_values.reserve(100);
            command_buffers.reserve(100);
        }
        std::vector<OneShotSubmissionPacket>  pkts;
        std::vector<uint64_t>                 done_signal_values;
        std::vector<vk::Semaphore>            done_semaphores;
        std::vector<vk::CommandBuffer>        command_buffers;

    };

    class OneShotSubmissionHandler
    {
        std::shared_ptr<vkr::Device>              _device;

        DeviceContext&                            _deviceContext;
        Queue                                     _copyQueue;
        Queue                                     _graphicsQueue;
        std::deque<OneShotSubmissionPacket>       _pendingOneShotCopies;
        std::deque<OneShotSubmissionPacket>       _pendingOneShotGraphics;

        ExecutingneShotSubmissions                _executingCopies{};
        ExecutingneShotSubmissions                _executingGraphics{};
        // TODO linear ranges with fixed capacity and holes fixing
        std::shared_ptr<SubmissionThreadExecutionContext> _submissionThreadExecutionContext;
        std::shared_ptr<FrameContext>            _frameContext;
        //future_t<void> DoAppendSubmission(vk::CommandBuffer buffer, std::deque<OneShotSubmissionPacket>& vec);
        //void DoSubmitPending(vk::Fence submitFence, Queue& queue, ExecutingneShotSubmissions& executing, std::deque<OneShotSubmissionPacket>& pending);
        //void DoPollCompletions(ExecutingneShotSubmissions& executing);
    public:
        OneShotSubmissionHandler(DeviceContext& deviceContext, Queue copyQueue, Queue graphicsQueue, std::shared_ptr<FrameContext> frameContext, std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext);
        future_t<void> AppendCopyCommandSubmission(vk::CommandBuffer buffer);
        void SubmitPendingCopies();
        void PollCopyCompletions();

        future_t<void> AppendGraphicsSubmission(vk::CommandBuffer buffer);
        void SubmitPendingGraphics();
        void PollGraphicsCompletions();

        [[nodiscard]] auto ResumeOnRecordingContext()
        {
            return concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());
        }

        SubmitCommandBuffer NewOneShotCopyCommandBuffer()
        {
            assert(tls_is_rendering_thread);
            return _frameContext->NewCopyCommandBuffer();
        }
        SubmitCommandBuffer NewOneShotGraphicsCommandBuffer()
        {
            assert(tls_is_rendering_thread);
            return _frameContext->NewGraphicsCommandBuffer();
        }
        Queue& CopyQueue()
        {
            return _copyQueue;
        }

        Queue& GraphicsQueue()
        {
            return _graphicsQueue;
        }



    };
}