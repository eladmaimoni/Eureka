#pragma once
#include "DeviceContext.hpp"

namespace eureka
{
    struct OneShotCopySubmissionPacket
    {
        vkr::CommandBuffer                 command_buffer{ nullptr };
        vkr::Semaphore                     done_timeline_semaphore{ nullptr };
        concurrencpp::result_promise<void> done_promise;
    };

    class OneShotCopySubmissionHandler
    {
        DeviceContext&                            _deviceContext;
        Queue                                     _copyQueue;
        std::deque<OneShotCopySubmissionPacket>   _pendingOneShotCopies;

        // TODO linear ranges with fixed capacity and holes fixing
        std::vector<OneShotCopySubmissionPacket>  _executingOneShotCopies;
        std::vector<uint64_t>                     _executingOneShotSignalValues;
        std::vector<vk::Semaphore>                _executingOneShotSignalSemaphores;
        std::vector<vk::CommandBuffer>            _executingOneShotCommandBuffers;
    public:
        OneShotCopySubmissionHandler(DeviceContext& deviceContext, Queue copyQueue);
        future_t<void> AppendOneShotCommandBufferSubmission(vkr::CommandBuffer buffer);

        void PollPendingOneShotSubmissions();

        void PollDoneOneShotSubmissions();
    };
}