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
        _executingOneShotCopies.reserve(100);
        _executingOneShotSignalValues.reserve(100);
        _executingOneShotSignalSemaphores.reserve(100);
    }

    future_t<void> OneShotSubmissionHandler::AppendOneShotCopyCommandBufferSubmission(vk::CommandBuffer buffer)
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

        OneShotCopySubmissionPacket sumbissionPacket
        {
            .command_buffer = buffer,
            .done_timeline_semaphore = _deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo)
        };

        auto result = sumbissionPacket.done_promise.get_result();

        _pendingOneShotCopies.emplace_back(std::move(sumbissionPacket));

        return result;
    }



    void OneShotSubmissionHandler::SubmitPendingOneShotCopies(vk::Fence signalFence)
    {


        if (!_pendingOneShotCopies.empty())
        {
            // TODO: find the first linear range that can be reused
            auto currentExecutingCount = _executingOneShotCopies.size();
            auto addedExecutingCount = static_cast<uint32_t>(std::min(_pendingOneShotCopies.size(), _executingOneShotCopies.capacity() - currentExecutingCount));

            for (auto i = 0u; i < addedExecutingCount; ++i)
            {
                auto pkt = std::move(_pendingOneShotCopies.front());
                _pendingOneShotCopies.pop_front();

                _executingOneShotSignalValues.emplace_back(1);
                _executingOneShotSignalSemaphores.emplace_back(*pkt.done_timeline_semaphore);
                _executingOneShotCommandBuffers.emplace_back(pkt.command_buffer);
                _executingOneShotCopies.emplace_back(std::move(pkt));
            }

            vk::TimelineSemaphoreSubmitInfo timelineInfo
            {
                .signalSemaphoreValueCount = addedExecutingCount,
                .pSignalSemaphoreValues = _executingOneShotSignalValues.data() + currentExecutingCount,
            };

            vk::SubmitInfo uploadsSubmitInfo
            {
                .pNext = &timelineInfo,
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = {},
                .commandBufferCount = addedExecutingCount,
                .pCommandBuffers = _executingOneShotCommandBuffers.data() + currentExecutingCount,
                .signalSemaphoreCount = addedExecutingCount,
                .pSignalSemaphores = _executingOneShotSignalSemaphores.data() + currentExecutingCount
            };

            _copyQueue->submit(uploadsSubmitInfo, signalFence);
        }
    }

    void OneShotSubmissionHandler::PollDoneOneShotSubmissions()
    {
        if (!_executingOneShotCopies.empty())
        {
            svec5<vk::Semaphore> waitSemaphores;
            auto totalExecuting = 0;
            auto totalDone = 0;
            for (auto i = 0; i < _executingOneShotCopies.size(); ++i)
            {
                if (_executingOneShotSignalValues[i] == 1)
                {
                    ++totalExecuting;
                    vk::SemaphoreWaitInfo waitInfo
                    {
                        .semaphoreCount = 1,
                        .pSemaphores = &_executingOneShotSignalSemaphores[i],
                        .pValues = &_executingOneShotSignalValues[i]
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

                        _executingOneShotCopies[i].done_promise.set_result();
                        _executingOneShotSignalValues[i] = 0;
                        _executingOneShotCopies[i] = {}; // destroy
                    }

                }
            }

            if (totalExecuting == totalDone)
            {
                _executingOneShotCopies.clear();
                _executingOneShotSignalValues.clear();
                _executingOneShotSignalSemaphores.clear();
                _executingOneShotCommandBuffers.clear();
            }
        }
    }

}