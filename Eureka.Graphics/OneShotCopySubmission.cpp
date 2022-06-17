#include "OneShotCopySubmission.hpp"
#include "SubmissionThreadExecutionContext.hpp"

namespace eureka
{
    static constexpr uint64_t DONE_VAL = 1;

    void DoPollCompletions(const vkr::Device& device, ExecutingneShotSubmissions& executing)
    {
        if (!executing.pkts.empty())
        {
            svec5<vk::Semaphore> waitSemaphores;
            auto totalExecuting = 0;
            auto totalDone = 0;
            for (auto i = 0; i < executing.pkts.size(); ++i)
            {
                if (executing.done_signal_values[i] == DONE_VAL)
                {
                    ++totalExecuting;
                    vk::SemaphoreWaitInfo waitInfo
                    {
                        .semaphoreCount = 1,
                        .pSemaphores = &executing.done_semaphores[i],
                        .pValues = &executing.done_signal_values[i]
                    };

                    auto result = device.waitSemaphores(waitInfo, 0);

                    if (result == vk::Result::eTimeout)
                    {
                        DEBUGGER_TRACE("pending semaphore {} not yet finished", i);
                    }
                    else if (result == vk::Result::eSuccess)
                    {
                        ++totalDone;
                        DEBUGGER_TRACE("pending semaphore {} done", i);

                        executing.pkts[i].done_promise.set_result();
                        executing.done_signal_values[i] = 0;
                        executing.pkts[i] = {}; // destroy
                    }

                }
            }

            if (totalExecuting == totalDone)
            {
                executing.pkts.clear();
                executing.command_buffers.clear();
                executing.done_semaphores.clear();
                executing.done_signal_values.clear();
            }
        }
    }


    void DoSubmitPending(vk::Fence submitFence, Queue& queue, ExecutingneShotSubmissions& executing, std::deque<OneShotSubmissionPacket>& pending)
    {
        auto currentExecutingCount = executing.pkts.size();
        auto addedExecutingCount = static_cast<uint32_t>(std::min(pending.size(), executing.command_buffers.capacity() - currentExecutingCount));

        for (auto i = 0u; i < addedExecutingCount; ++i)
        {
            auto pkt = std::move(pending.front());
            pending.pop_front();

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


        queue->submit(uploadsSubmitInfo, submitFence);

    }

    future_t<void> DoAppendSubmission(const vkr::Device& device, vk::CommandBuffer buffer, std::deque<OneShotSubmissionPacket>& vec)
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
            .done_timeline_semaphore = device.createSemaphore(semaphoreCreateInfo)
        };

        auto result = sumbissionPacket.done_promise.get_result();

        vec.emplace_back(std::move(sumbissionPacket));

        return result;
    }

    OneShotSubmissionHandler::OneShotSubmissionHandler(
        DeviceContext& deviceContext,
        Queue copyQueue,
        Queue graphicsQueue,
        std::shared_ptr<FrameContext> frameContext,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext
    ) :
        _device(deviceContext.LogicalDevice()),
        _deviceContext(deviceContext),
        _frameContext(std::move(frameContext)),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue)
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////
    
    void OneShotSubmissionHandler::PollCopyCompletions()
    {
        DoPollCompletions(*_device, _executingCopies);
    }

    void OneShotSubmissionHandler::SubmitPendingCopies()
    {
        if (!_pendingOneShotCopies.empty())
        {
            DoSubmitPending(_frameContext->NewCopySubmitFence(), _copyQueue, _executingCopies, _pendingOneShotCopies);
        }
    }


    future_t<void> OneShotSubmissionHandler::AppendCopyCommandSubmission(vk::CommandBuffer buffer)
    {
        return DoAppendSubmission(*_device, buffer, _pendingOneShotCopies);
    }



    //////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////

    void OneShotSubmissionHandler::PollGraphicsCompletions()
    {
        DoPollCompletions(*_device, _executingGraphics);
    }

    void OneShotSubmissionHandler::SubmitPendingGraphics()
    {
        if (!_pendingOneShotGraphics.empty())
        {
            DoSubmitPending(_frameContext->NewGraphicsSubmitFence(), _graphicsQueue, _executingGraphics, _pendingOneShotGraphics);
        }
    }

    future_t<void> OneShotSubmissionHandler::AppendGraphicsSubmission(vk::CommandBuffer buffer)
    {
        return DoAppendSubmission(*_device, buffer, _pendingOneShotGraphics);
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////






}