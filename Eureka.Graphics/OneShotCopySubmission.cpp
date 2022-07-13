#include "OneShotCopySubmission.hpp"
#include "SubmissionThreadExecutionContext.hpp"

namespace eureka
{



    OneShotSubmissionHandler::OneShotSubmissionHandler(
        DeviceContext& deviceContext,
        Queue copyQueue,
        Queue graphicsQueue,
        std::shared_ptr<SwapChainFrameContext> frameContext,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext
    ) :
        _deviceContext(deviceContext),
        _frameContext(std::move(frameContext)),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue)
    {

    }

    future_t<void> OneShotSubmissionHandler::AppendCopyCommandSubmission(vk::CommandBuffer buffer)
    {
        return DoAppendSubmission(buffer, _pendingOneShotCopies);
    }

    future_t<void> OneShotSubmissionHandler::AppendGraphicsSubmission(vk::CommandBuffer buffer)
    {
        return DoAppendSubmission(buffer, _pendingOneShotGraphics);
    }

    void OneShotSubmissionHandler::SubmitPendingGraphics(vk::Fence signalFence)
    {
        DoSubmitPending(signalFence, _graphicsQueue, _executingGraphics, _pendingOneShotGraphics);
    }

    void OneShotSubmissionHandler::PollraphicsCompletions()
    {
        DoPollCompletions(_executingGraphics);
    }

    future_t<void> OneShotSubmissionHandler::DoAppendSubmission(vk::CommandBuffer buffer, std::deque<OneShotSubmissionPacket>& vec)
    {
        assert(tls_is_rendering_thread);

        vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo
        {
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue = 0
        };

        vk::SemaphoreCreateInfo semaphoreCreateInfo
        {
            .pNext = &semaphoreTypeCreateInfo
        };

        OneShotSubmissionPacket sumbissionPacket
        {
            .command_buffer = buffer,
            .done_timeline_semaphore = _deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo)
        };

        auto result = sumbissionPacket.done_promise.get_result();

        vec.emplace_back(std::move(sumbissionPacket));

        return result;
    }

    void OneShotSubmissionHandler::DoSubmitPending(vk::Fence signalFence, Queue& queue, ExecutingneShotSubmissions& executing, std::deque<OneShotSubmissionPacket>& pending)
    {
        if (!pending.empty())
        {
            auto currentExecutingCount = executing.pkts.size();
            auto addedExecutingCount = static_cast<uint32_t>(std::min(pending.size(), executing.command_buffers.capacity() - currentExecutingCount));

            for (auto i = 0u; i < addedExecutingCount; ++i)
            {
                auto pkt = std::move(_pendingOneShotCopies.front());
                _pendingOneShotCopies.pop_front();

                executing.done_signal_values.emplace_back(DONE_VAL);
                executing.done_semaphores.emplace_back(*pkt.done_timeline_semaphore);
                executing.command_buffers.emplace_back(pkt.command_buffer);
                executing.pkts.emplace_back(std::move(pkt));
            }

            vk::TimelineSemaphoreSubmitInfo timelineInfo
            {
                .signalSemaphoreValueCount = addedExecutingCount,
                .pSignalSemaphoreValues = executing.done_signal_values.data() + currentExecutingCount,
            };

            vk::SubmitInfo uploadsSubmitInfo
            {
                .pNext = &timelineInfo,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = {},
                .commandBufferCount = addedExecutingCount,
                .pCommandBuffers = executing.command_buffers.data() + currentExecutingCount,
                .signalSemaphoreCount = addedExecutingCount,
                .pSignalSemaphores = executing.done_semaphores.data() + currentExecutingCount
            };

            queue->submit(uploadsSubmitInfo, signalFence);
        }
    }

    void OneShotSubmissionHandler::SubmitPendingCopies(vk::Fence signalFence)
    {
        DoSubmitPending(signalFence, _copyQueue, _executingCopies, _pendingOneShotCopies);
    }

    void OneShotSubmissionHandler::PollCopyCompletions()
    {
        DoPollCompletions(_executingCopies);
    }

    void OneShotSubmissionHandler::DoPollCompletions(ExecutingneShotSubmissions& executing)
    {
        if (!_executingCopies.pkts.empty())
        {
            svec5<vk::Semaphore> waitSemaphores;
            auto totalExecuting = 0;
            auto totalDone = 0;
            for (auto i = 0; i < _executingCopies.pkts.size(); ++i)
            {
                if (_executingCopies.done_signal_values[i] == DONE_VAL)
                {
                    ++totalExecuting;
                    vk::SemaphoreWaitInfo waitInfo
                    {
                        .semaphoreCount = 1,
                        .pSemaphores = &executing.done_semaphores[i],
                        .pValues = &executing.done_signal_values[i]
                    };

                    auto result = _deviceContext.LogicalDevice()->waitSemaphores(waitInfo, 0);

                    if (result == vk::Result::eTimeout)
                    {
                        DEBUGGER_TRACE("pending semaphore {} not yet finished", i);
                    }
                    else if (result == vk::Result::eSuccess)
                    {
                        ++totalDone;
                        DEBUGGER_TRACE("pending semaphore {} done", i);

                        _executingCopies.pkts[i].done_promise.set_result();
                        _executingCopies.done_signal_values[i] = 0;
                        _executingCopies.pkts[i] = {}; // destroy
                    }

                }
            }

            if (totalExecuting == totalDone)
            {
                _executingCopies.pkts.clear();
                _executingCopies.command_buffers.clear();
                _executingCopies.done_semaphores.clear();
                _executingCopies.done_signal_values.clear();
            }
        }
    }

}