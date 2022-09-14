#include "SubmissionThreadExecutionContext.hpp"


namespace eureka
{

    SubmissionThreadExecutionContext::SubmissionThreadExecutionContext(
        DeviceContext& deviceContext, 
        Queue copyQueue, 
        Queue graphicsQueue,
        SubmissionThreadExecutor executor
    ) :
        _deviceConetext(deviceContext),
        _copyQueue(copyQueue),
        _graphicsQueue(graphicsQueue),
        _executor(std::move(executor)),
        _oneShotCopyCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
    {

    }

    SubmissionThreadExecutionContext::~SubmissionThreadExecutionContext()
    {

    }

}