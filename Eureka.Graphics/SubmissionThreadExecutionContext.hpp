#pragma once
#include "DeviceContext.hpp"
#include "SubmissionThreadExecutor.hpp"
#include "Commands.hpp"

namespace eureka
{
    using SubmissionThreadExecutor = std::shared_ptr<submission_thread_executor>;

    struct OneShotCopySubmissionPacket
    {
        vkr::CommandBuffer                 command_buffer{ nullptr };
        vkr::Fence                         done_fence{ nullptr };
        concurrencpp::result_promise<void> done_promise;
    };

    class SubmissionThreadExecutionContext
    {
    private:
        DeviceContext&                             _deviceConetext;
        Queue                                      _copyQueue;
        SubmissionThreadExecutor                   _executor;
        CommandPool                                _oneShotCopyCommandPool;
        std::vector<OneShotCopySubmissionPacket>   _oneShotCopyCommandBuffers;
    public:
        SubmissionThreadExecutionContext(
            DeviceContext& deviceContext,
            Queue copyQueue,
            SubmissionThreadExecutor executor
        )
            :
            _deviceConetext(deviceContext),
            _copyQueue(copyQueue),
            _executor(std::move(executor)),
            _oneShotCopyCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
        {

        }

        CommandPool& OneShotCopySubmitCommandPool()
        {
            // TODO assert correct thread
            return _oneShotCopyCommandPool;
        }

        concurrencpp::result<void> AppendOneShotCommandBufferSubmission(vkr::CommandBuffer buffer)
        {
            // TODO assert correct thread
            OneShotCopySubmissionPacket sumbissionPacket
            {
                .command_buffer = std::move(buffer),
                .done_fence = _deviceConetext.LogicalDevice()->createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled }),
            };

            auto result = sumbissionPacket.done_promise.get_result();

            _oneShotCopyCommandBuffers.emplace_back(std::move(sumbissionPacket));
        }

        submission_thread_sub_executor& OneShotCopySubmitExecutor()
        {
            return _executor->one_shot_copy_submit_executor();
        }

        submission_thread_executor& Executor()
        {
            return *_executor;
        }


    };
}