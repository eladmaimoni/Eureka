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

        submission_thread_sub_executor& OneShotCopySubmitExecutor()
        {
            return _executor->one_shot_copy_submit_executor();
        }
        submission_thread_sub_executor& PreRenderExecutor()
        {
            return _executor->pre_render_executor();
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

    };
}