#pragma once

#include "DeviceContext.hpp"
#include "Buffer.hpp"
#include "GraphicsDefaults.hpp"
#include "Commands.hpp"


















namespace eureka
{
    struct StageZoneConfig
    {
        uint64_t bytes_capacity;
    };

    class SequentialStageZone
    {
    private:
        HostWriteCombinedBuffer _buffer;
        uint64_t               _position{ 0 };

    public:
        SequentialStageZone(DeviceContext& deviceContext, StageZoneConfig config)
            : _buffer(deviceContext, BufferConfig{ .byte_size = config.bytes_capacity })
        {


        }
        uint64_t Position() const
        {
            return _position;
        }
        uint64_t LeftoverBytes() const
        {
            return _buffer.ByteSize() - _position;
        }

        template<typename T, std::size_t COUNT>
        void Assign(std::span<T, COUNT> s)
        {
            assert(s.size_bytes() <= LeftoverBytes());
            _buffer.Assign(s, _position);
            _position += s.size_bytes();
        }

        void Reset()
        {
            _position = 0;
        }



    };


    struct OneShotCopySubmissionPacket
    {
        vkr::CommandBuffer                 command_buffer{ nullptr };
        vkr::Fence                         done_fence{nullptr};
        concurrencpp::result_promise<void> done_promise;
    };


    
    class SubmissionThreadContext
    {
    private:
        DeviceContext& _deviceConetext;
        Queue                                     _copyQueue;
        OneShotCopySubmitExecutor                 _oneShotCopySubmitExecutor;
        CommandPool                               _oneShotCopyCommandPool;
        std::vector<OneShotCopySubmissionPacket>  _oneShotCopyCommandBuffers;
    public:
        SubmissionThreadContext(
            DeviceContext& deviceContext,
            Queue queue,
            OneShotCopySubmitExecutor oneShotCopySubmitExecutor
            )
            : 
            _deviceConetext(deviceContext),
            _copyQueue(queue),
            _oneShotCopySubmitExecutor(std::move(oneShotCopySubmitExecutor)),
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

        ManualExecutor& OneShotCopySubmitExecutionContext()
        {
            return *_oneShotCopySubmitExecutor;
        }
    };


}