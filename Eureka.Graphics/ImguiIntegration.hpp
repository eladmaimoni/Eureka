#pragma once
#include <imgui.h>
#include "DeviceContext.hpp"
#include "Pool.hpp"
#include "Image.hpp"
#include "Commands.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "UploadRingBuffer.hpp"
#include "GraphicsDefaults.hpp"
#include "CommandsUtils.hpp"
#include "Pipeline.hpp"

namespace eureka
{
	class ImGuiIntegration
	{
		DeviceContext& _deviceContext;
		std::shared_ptr<HostWriteCombinedRingPool> _uploadPool;
		PoolExecutor _poolExecutor;
		std::shared_ptr<SubmissionThreadExecutionContext> _submissionThreadExecutionContext;
		std::shared_ptr<OneShotCopySubmissionHandler> _oneShotCopySubmissionHandler;

	public:
		ImGuiIntegration(
			DeviceContext& deviceContext,
			std::shared_ptr<PipelineCache> pipelineCache,
			std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
			std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
			std::shared_ptr<HostWriteCombinedRingPool> uploadPool,			
			PoolExecutor poolExecutor
			);

		future_t<void> Setup(std::shared_ptr<PipelineCache> pipelineCache);

		~ImGuiIntegration();
	};

}

