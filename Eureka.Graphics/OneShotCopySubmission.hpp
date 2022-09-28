#pragma once
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/FrameContext.hpp"
#include "../Eureka.Vulkan/Synchronization.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include <boost/container/stable_vector.hpp>


namespace eureka::graphics
{
    template<typename T>
    using stable_vec = boost::container::stable_vector<T>;

    struct OneShotSubmissionWait
    {
        vulkan::CounterSemaphoreHandle semaphore;
        VkPipelineStageFlags stages;
    };

    struct OneShotSubmissionPacket
    {
        VkCommandBuffer                        command_buffer{ nullptr };
        vulkan::CounterSemaphoreHandle         signal;
        svec5<OneShotSubmissionWait>           wait_list;
        concurrencpp::result_promise<void>     done_promise;
    };

    struct ExecutingnOneShotSubmissionBatch
    {
        svec5<VkCommandBuffer>                         command_buffers;
        svec5<vulkan::CounterSemaphoreHandle>          wait_semaphores;
        svec5<VkPipelineStageFlags>                    wait_stages;
        uint64_t                                       done_signal_value;
        svec5<vulkan::CounterSemaphoreHandle>          signal_list;
        svec5<promise_t<void>>                         done_promises;
    };

    class OneShotSubmissionHandler
    {
        std::shared_ptr<vulkan::Device>                     _device;
        vulkan::Queue                                       _copyQueue;
        vulkan::Queue                                       _graphicsQueue;
        std::vector<OneShotSubmissionPacket>                _pendingOneShotCopies;
        std::vector<OneShotSubmissionPacket>                _pendingOneShotGraphics;

        stable_vec<ExecutingnOneShotSubmissionBatch>        _executingCopies{};
        stable_vec<ExecutingnOneShotSubmissionBatch>        _executingGraphics{};
        // TODO linear ranges with fixed capacity and holes fixing
        std::shared_ptr<SubmissionThreadExecutionContext>   _submissionThreadExecutionContext;
        std::shared_ptr<vulkan::FrameContext>               _frameContext;
    public:
        OneShotSubmissionHandler(
            std::shared_ptr<vulkan::Device> device, 
            vulkan::Queue copyQueue, 
            vulkan::Queue graphicsQueue, 
            std::shared_ptr<vulkan::FrameContext> frameContext, 
            std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext
        );
        future_t<void> AppendCopyCommandSubmission(vulkan::LinearCommandBufferHandle buffer, vulkan::CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList = {});
        void SubmitPendingCopies();
        void PollCopyCompletions();
        future_t<void> AppendGraphicsSubmission(vulkan::LinearCommandBufferHandle buffer, vulkan::CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList = {});

        void SubmitPendingGraphics();
        void PollGraphicsCompletions();

        [[nodiscard]] auto ResumeOnRecordingContext()
        {
            return concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());
        }

        vulkan::SubmitCommandBuffer NewOneShotCopyCommandBuffer()
        {
            assert(tls_is_rendering_thread);
            return _frameContext->NewCopyCommandBuffer();
        }
        vulkan::SubmitCommandBuffer NewOneShotGraphicsCommandBuffer()
        {
            assert(tls_is_rendering_thread);
            return _frameContext->NewGraphicsCommandBuffer();
        }
        vulkan::Queue& CopyQueue()
        {
            return _copyQueue;
        }

        vulkan::Queue& GraphicsQueue()
        {
            return _graphicsQueue;
        }



    };
}