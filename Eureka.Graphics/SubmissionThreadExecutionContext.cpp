#include "SubmissionThreadExecutionContext.hpp"


namespace eureka::graphics
{

    SubmissionThreadExecutionContext::SubmissionThreadExecutionContext(
        std::shared_ptr<vulkan::Device> device,
        vulkan::Queue copyQueue,
        vulkan::Queue graphicsQueue,
        SubmissionThreadExecutor executor
    ) :
        _device(std::move(device)),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue),
        _executor(std::move(executor))
        //_oneShotCopyCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
    {

    }

    SubmissionThreadExecutionContext::~SubmissionThreadExecutionContext()
    {

    }

}