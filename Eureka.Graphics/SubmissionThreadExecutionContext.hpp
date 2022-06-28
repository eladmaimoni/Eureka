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
        vkr::Semaphore                     done_timeline_semaphore{ nullptr };
        concurrencpp::result_promise<void> done_promise;
    };

    class SubmissionThreadExecutionContext
    {
    private:
        DeviceContext&                             _deviceConetext;
        Queue                                      _copyQueue;
        Queue                                      _graphicsQueue;
        SubmissionThreadExecutor                   _executor;
        CommandPool                                _oneShotCopyCommandPool;
        std::deque<OneShotCopySubmissionPacket>   _oneShotCopyCommandBuffers;
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

        concurrencpp::result<void> AppendOneShotCommandBufferSubmission(vkr::CommandBuffer buffer)
        {
            vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo
            {
                .semaphoreType = vk::SemaphoreType::eTimeline,
                .initialValue = 0
            };

            vk::SemaphoreCreateInfo semaphoreCreateInfo
            { 
                .pNext = &semaphoreTypeCreateInfo
            };

            // TODO assert correct thread
            OneShotCopySubmissionPacket sumbissionPacket
            {
                .command_buffer = std::move(buffer),
                .done_timeline_semaphore = _deviceConetext.LogicalDevice()->createSemaphore(semaphoreCreateInfo)               
            };

            auto result = sumbissionPacket.done_promise.get_result();

            _oneShotCopyCommandBuffers.emplace_back(std::move(sumbissionPacket));

            return result;
        }

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

        std::size_t OneShotCopySubmissionPacketsCount()
        {
            // TODO assert correct thread
            return _oneShotCopyCommandBuffers.size();
        }

        auto RetrieveOneShotCopySubmissionPackets(std::size_t count)
        {
            // TODO assert correct thread
            svec10<OneShotCopySubmissionPacket> pkts(count);
            // std::ranges::move(_oneShotCopyCommandBuffers | std::ranges::views::take(count), std::back_inserter(pkts));

            // TODO probably a better way to move the first count elements
            for (auto i = 0; i < count; ++i)
            {
                pkts[i] = std::move(_oneShotCopyCommandBuffers.front());
                _oneShotCopyCommandBuffers.pop_front();
            }

            return pkts;
        }
    };
}