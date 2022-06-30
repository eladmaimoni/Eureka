#pragma once
#include "DeviceContext.hpp"
#include "SubmissionThreadExecutor.hpp"
#include "Commands.hpp"


namespace eureka
{
    using SubmissionThreadExecutor = std::shared_ptr<submission_thread_executor>;

    inline thread_local bool tls_is_rendering_thread = false;

    class SubmissionThreadExecutionContext
    {
    private:
        DeviceContext&                             _deviceConetext;
        Queue                                      _copyQueue;
        Queue                                      _graphicsQueue;
        SubmissionThreadExecutor                   _executor;
        CommandPool                                _oneShotCopyCommandPool;
        //std::deque<OneShotCopySubmissionPacket>    _oneShotCopyCommandBuffers;

        
    public:
        SubmissionThreadExecutionContext(
            DeviceContext& deviceContext,
            Queue copyQueue,
            Queue graphicsQueue,
            SubmissionThreadExecutor executor
        )
            :
            _deviceConetext(deviceContext),
            _copyQueue(copyQueue),
            _graphicsQueue(graphicsQueue),
            _executor(std::move(executor)),
            _oneShotCopyCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
        {

        }

        Queue& CopyQueue()
        {
            return _copyQueue;
        }
        Queue& GraphicsQueue()
        {
            return _graphicsQueue;
        }
        CommandPool& OneShotCopySubmitCommandPool()
        {
            // TODO assert correct thread
            return _oneShotCopyCommandPool;
        }

        //concurrencpp::result<void> AppendOneShotCommandBufferSubmission(vkr::CommandBuffer buffer)
        //{


        //    auto result = sumbissionPacket.done_promise.get_result();

        //    _oneShotCopySumbissionHandler->AppendSumbissionPacket(std::move(sumbissionPacket));
        //    _oneShotCopyCommandBuffers.emplace_back(std::move(sumbissionPacket));

        //    return result;
        //}

        submission_thread_sub_executor& OneShotCopySubmitExecutor()
        {
            return _executor->one_shot_copy_submit_executor();
        }

        submission_thread_executor& Executor()
        {
            return *_executor;
        }

        // 
        // Rendering thread accessors
        //

        void SetCurrentThreadAsRenderingThread()
        {
            tls_is_rendering_thread = true;
        }

        //std::size_t OneShotCopySubmissionPacketsCount()
        //{
        //    assert(tls_is_rendering_thread);
        //    //return _oneShotCopyCommandBuffers.size();
        //}

        //auto RetrieveOneShotCopySubmissionPackets(std::size_t count)
        //{
        //    assert(tls_is_rendering_thread);
        //    svec10<OneShotCopySubmissionPacket> pkts(count);
        //    // std::ranges::move(_oneShotCopyCommandBuffers | std::ranges::views::take(count), std::back_inserter(pkts));

        //    // TODO probably a better way to move the first count elements
        //    for (auto i = 0; i < count; ++i)
        //    {
        //        pkts[i] = std::move(_oneShotCopyCommandBuffers.front());
        //        _oneShotCopyCommandBuffers.pop_front();
        //    }

        //    return pkts;
        //}
    };
}