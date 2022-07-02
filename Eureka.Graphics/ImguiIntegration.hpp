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

namespace eureka
{
	class ImGuiIntegration
	{
		DeviceContext& _deviceContext;
		std::shared_ptr<HostWriteCombinedRingPool> _uploadPool;
		PoolExecutor _poolExecutor;
	public:
		ImGuiIntegration(
			DeviceContext& deviceContext,
			std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
			std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
			std::shared_ptr<HostWriteCombinedRingPool> uploadPool,
			PoolExecutor poolExecutor
			)
			: _deviceContext(deviceContext), _uploadPool(std::move(uploadPool)), _poolExecutor(std::move(poolExecutor))
		{
			ImGui::CreateContext();
		}

		future_t<void> Setup()
		{
			ImGuiIO& io = ImGui::GetIO();

			// Create font texture
			unsigned char* fontData;
			int texWidth, texHeight;
			io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
			VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

			co_await concurrencpp::resume_on(*_poolExecutor);

			Image2DProperties fontImageProps
			{
				 .width = static_cast<uint32_t>(texWidth),
				 .height = static_cast<uint32_t>(texHeight),
				 .format = vk::Format::eR8G8B8A8Unorm,
				 .usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				 .aspect_flags = vk::ImageAspectFlagBits::eColor,
				 .use_dedicated_memory_allocation = false // unlikely to be resized
			};

			auto fontImage = SampledImage2D(_deviceContext, fontImageProps);

			auto stageBuffer = co_await _uploadPool->EnqueueAllocation(uploadSize);
		}

		~ImGuiIntegration()
		{
			ImGui::DestroyContext();
		}
	};

}

