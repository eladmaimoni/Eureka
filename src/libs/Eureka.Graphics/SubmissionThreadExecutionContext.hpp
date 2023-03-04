#pragma once
#include "../Eureka.Vulkan/Buffer.hpp"
//#include "../Eureka.Vulkan/Queue.hpp"

#include "SubmissionThreadExecutor.hpp"
//#include "Commands.hpp"


namespace eureka::graphics
{
    using SubmissionThreadExecutor = std::shared_ptr<submission_thread_executor>;

    inline thread_local bool tls_is_rendering_thread = false;

    class SubmissionThreadExecutionContext
    {
    private:
        std::shared_ptr<vulkan::Device>            _device;
        vulkan::Queue                              _copyQueue;
        vulkan::Queue                              _graphicsQueue;
        SubmissionThreadExecutor                   _executor;
        //CommandPool                                _oneShotCopyCommandPool;
    public:
        SubmissionThreadExecutionContext(
            std::shared_ptr<vulkan::Device> device,
            vulkan::Queue copyQueue,
            vulkan::Queue graphicsQueue,
            SubmissionThreadExecutor executor
        );

        ~SubmissionThreadExecutionContext();


        vulkan::Queue& CopyQueue()
        {
            return _copyQueue;
        }

        vulkan::Queue& GraphicsQueue()
        {
            return _graphicsQueue;
        }

        //vulkan::LinearCommandPool& OneShotCopySubmitCommandPool()
        //{
        //    // TODO assert correct thread
        //    return _oneShotCopyCommandPool;
        //}

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