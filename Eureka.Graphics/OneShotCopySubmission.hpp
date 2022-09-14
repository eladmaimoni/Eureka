#pragma once
#include "DeviceContext.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "FrameContext.hpp"
#include <boost/container/stable_vector.hpp>


namespace eureka
{
    template<typename T>
    using stable_vec = boost::container::stable_vector<T>;

    struct OneShotSubmissionWait
    {
        CounterSemaphoreHandle semaphore;
        vk::PipelineStageFlags stages;
    };

    struct OneShotSubmissionPacket
    {
        vk::CommandBuffer                      command_buffer{ nullptr };
        CounterSemaphoreHandle                 signal;
        svec5<OneShotSubmissionWait>           wait_list;
        concurrencpp::result_promise<void>     done_promise;
    };

    struct ExecutingnOneShotSubmissionBatch
    {
        svec5<vk::CommandBuffer>       command_buffers;
        svec5<CounterSemaphoreHandle>  wait_semaphores;
        svec5<vk::PipelineStageFlags>  wait_stages;
        uint64_t                       done_signal_value;
        svec5<CounterSemaphoreHandle>  signal_list;
        svec5<promise_t<void>>         done_promises;
    };

    class OneShotSubmissionHandler
    {
        std::shared_ptr<vkr::Device>              _device;

        DeviceContext&                            _deviceContext;
        Queue                                     _copyQueue;
        Queue                                     _graphicsQueue;
        std::vector<OneShotSubmissionPacket>      _pendingOneShotCopies;
        std::vector<OneShotSubmissionPacket>      _pendingOneShotGraphics;

        stable_vec<ExecutingnOneShotSubmissionBatch>    _executingCopies{};
        stable_vec<ExecutingnOneShotSubmissionBatch>    _executingGraphics{};
        // TODO linear ranges with fixed capacity and holes fixing
        std::shared_ptr<SubmissionThreadExecutionContext> _submissionThreadExecutionContext;
        std::shared_ptr<FrameContext>                     _frameContext;
    public:
        OneShotSubmissionHandler(DeviceContext& deviceContext, Queue copyQueue, Queue graphicsQueue, std::shared_ptr<FrameContext> frameContext, std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext);
        future_t<void> AppendCopyCommandSubmission(vk::CommandBuffer buffer, CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList = {});
        void SubmitPendingCopies();
        void PollCopyCompletions();

        future_t<void> AppendGraphicsSubmission(vk::CommandBuffer buffer, CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList = {});
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