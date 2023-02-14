#include "OneShotCopySubmission.hpp"
#include "SubmissionThreadExecutionContext.hpp"

namespace eureka::graphics
{
    //static constexpr uint64_t DONE_VAL = 1;

    void DoPollCompletions(stable_vec<ExecutingnOneShotSubmissionBatch>& executing_vec)
    {
        if (!executing_vec.empty())
        {
            auto removeRange = std::ranges::remove_if(
                executing_vec,
                [&](ExecutingnOneShotSubmissionBatch& batch)
                {
                    if (std::ranges::all_of(batch.signal_list, [](vulkan::CounterSemaphoreHandle& sem)
                        {
                            return sem.Query();
                        }))
                    {
                        for (auto& pr : batch.done_promises)
                        {
                            pr.set_result();
                        }

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            );
            executing_vec.erase(removeRange.begin(), removeRange.end());
        }
    }

    void DoSubmitPending(
        VkFence submitFence, 
        vulkan::Queue& queue, 
        stable_vec<ExecutingnOneShotSubmissionBatch>& executing_vec, 
        std::vector<OneShotSubmissionPacket>& pending_vec
    )
    {
        auto& new_batch = executing_vec.emplace_back();

        for (auto& pending : pending_vec)
        {
            new_batch.command_buffers.emplace_back(pending.command_buffer);
            new_batch.signal_list.emplace_back(pending.signal);
            new_batch.done_promises.emplace_back(std::move(pending.done_promise));
            for (auto wait : pending.wait_list)
            {
                new_batch.wait_semaphores.emplace_back(wait.semaphore);
                new_batch.wait_stages.emplace_back(wait.stages);
            }
        }
        pending_vec.clear();

        svec5<VkSemaphore> signalSemapores(new_batch.signal_list.size());
        svec5<uint64_t> signalValues(new_batch.signal_list.size());
        svec5<VkSemaphore> waitSemapores(new_batch.wait_semaphores.size());
        svec5<uint64_t> waitValues(new_batch.wait_semaphores.size());

        for (auto i = 0u; i < new_batch.wait_semaphores.size(); ++i)
        {
            auto& wait = new_batch.wait_semaphores[i];
            
            waitSemapores[i] = wait.Get();
            waitValues[i] = wait.Value();
        }

        for (auto i = 0u; i < new_batch.signal_list.size(); ++i)
        {
            auto& signal = new_batch.signal_list[i];
            auto prev = signal.Increment();
            signalSemapores[i] = signal.Get();
            signalValues[i] = prev + 1;
        }

        VkTimelineSemaphoreSubmitInfo timelineInfo
        { 
            .sType = VkStructureType::VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .waitSemaphoreValueCount = static_cast<uint32_t>(waitSemapores.size()),
            .pWaitSemaphoreValues = waitValues.data(),
            .signalSemaphoreValueCount = static_cast<uint32_t>(signalSemapores.size()),
            .pSignalSemaphoreValues = signalValues.data()
        };


        // by default we unify multiple command buffers to a single submission
        VkSubmitInfo uploadsSubmitInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = &timelineInfo,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemapores.size()),
            .pWaitSemaphores = waitSemapores.data(),
            .pWaitDstStageMask = new_batch.wait_stages.data(),
            .commandBufferCount = static_cast<uint32_t>(new_batch.command_buffers.size()),
            .pCommandBuffers = new_batch.command_buffers.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemapores.size()),
            .pSignalSemaphores = signalSemapores.data()
        };

        queue.Submit(uploadsSubmitInfo, submitFence);

    }

    future_t<void> DoAppendSubmission(vulkan::LinearCommandBufferHandle buffer, vulkan::CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList, std::vector<OneShotSubmissionPacket>& vec)
    {
        assert(tls_is_rendering_thread);

        auto& sumbissionPacket = vec.emplace_back();

        sumbissionPacket.command_buffer = buffer.Get();
        sumbissionPacket.signal = std::move(signal);

        for (auto wait : waitList)
        {
            sumbissionPacket.wait_list.emplace_back(wait);
        }

        return sumbissionPacket.done_promise.get_result();
    }

    OneShotSubmissionHandler::OneShotSubmissionHandler(
        std::shared_ptr<vulkan::Device> device,
        vulkan::Queue copyQueue,
        vulkan::Queue graphicsQueue,
        std::shared_ptr<vulkan::FrameContext> frameContext,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext
    ) :
        _device(std::move(device)),
        _frameContext(std::move(frameContext)),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue)
    {
        _executingCopies.reserve(20);
        _executingGraphics.reserve(20);
    }
    
    void OneShotSubmissionHandler::PollCopyCompletions()
    {
        DoPollCompletions(_executingCopies);
    }

    void OneShotSubmissionHandler::SubmitPendingCopies()
    {
        if (!_pendingOneShotCopies.empty())
        {
            DoSubmitPending(_frameContext->NewCopySubmitFence(), _copyQueue, _executingCopies, _pendingOneShotCopies);
        }
    }


    future_t<void> OneShotSubmissionHandler::AppendCopyCommandSubmission(vulkan::LinearCommandBufferHandle buffer, vulkan::CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList)
    {
        return DoAppendSubmission(buffer, std::move(signal), waitList, _pendingOneShotCopies);
    }



    //////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////

    void OneShotSubmissionHandler::PollGraphicsCompletions()
    {
        DoPollCompletions(_executingGraphics);
    }

    void OneShotSubmissionHandler::SubmitPendingGraphics()
    {
        if (!_pendingOneShotGraphics.empty())
        {
            DoSubmitPending(_frameContext->NewGraphicsSubmitFence(), _graphicsQueue, _executingGraphics, _pendingOneShotGraphics);
        }
    }

    future_t<void> OneShotSubmissionHandler::AppendGraphicsSubmission(vulkan::LinearCommandBufferHandle buffer, vulkan::CounterSemaphoreHandle signal, dynamic_span<OneShotSubmissionWait> waitList)
    {
        return DoAppendSubmission(buffer, std::move(signal), waitList, _pendingOneShotGraphics);
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////






}