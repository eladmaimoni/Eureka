#include "ImguiIntegration.hpp"

namespace eureka
{


    ImGuiIntegration::ImGuiIntegration(DeviceContext& deviceContext, std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext, std::shared_ptr<OneShotCopySubmissionHandler> oneShotCopySubmissionHandler, std::shared_ptr<HostWriteCombinedRingPool> uploadPool, PoolExecutor poolExecutor) :
        _deviceContext(deviceContext),
        _uploadPool(std::move(uploadPool)),
        _poolExecutor(std::move(poolExecutor)),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _oneShotCopySubmissionHandler(std::move(oneShotCopySubmissionHandler))
    {
        ImGui::CreateContext();

    }

    future_t<void> ImGuiIntegration::Setup(std::shared_ptr<PipelineCache> pipelineCache)
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("imgui setup", Profiling::Color::Red, Profiling::PROFILING_CATEGORY_INIT);
        co_await concurrencpp::resume_on(*_poolExecutor);

        ImGuiIO& io = ImGui::GetIO();

        // Create font texture
        unsigned char* fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

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

        ImageStageUploadDesc transferDesc
        {
            .unpinned_src_span = std::span(fontData, uploadSize),
            .stage_zone_offset = 0,
            .destination_image = fontImage.Get(),
            .destination_image_extent = vk::Extent3D{.width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight), .depth = 1}
        };

        auto stageBuffer = co_await _uploadPool->EnqueueAllocation(uploadSize);


        stageBuffer.Assign(transferDesc.unpinned_src_span, 0);

        auto [preTransferBarrier, bufferImageCopy, postTransferBarrier] =
            ShaderSampledImageUploadTuple(
                _submissionThreadExecutionContext->CopyQueue(),
                _submissionThreadExecutionContext->GraphicsQueue(),
                transferDesc
            );


        auto imguiPipline = pipelineCache->GetImGuiPipeline();


        co_await concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());

        auto uploadCommandBuffer = _submissionThreadExecutionContext->OneShotCopySubmitCommandPool().AllocatePrimaryCommandBuffer();
        {
            PROFILE_CATEGORIZED_SCOPE("imgui commands", Profiling::Color::Green, Profiling::PROFILING_CATEGORY_RENDERING);
            ScopedCommands commands(uploadCommandBuffer);

            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                {},
                nullptr,
                nullptr,
                { preTransferBarrier }
            );

            uploadCommandBuffer.copyBufferToImage(
                stageBuffer.Buffer(),
                transferDesc.destination_image,
                vk::ImageLayout::eTransferDstOptimal,
                { bufferImageCopy }
            );

            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                nullptr,
                nullptr,
                { postTransferBarrier }
            );
        }

        auto fut = _oneShotCopySubmissionHandler->AppendOneShotCommandBufferSubmission(std::move(uploadCommandBuffer));



        co_await fut;

        co_await concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());

        co_return;
    }

    ImGuiIntegration::~ImGuiIntegration()
    {
        ImGui::DestroyContext();
    }

}


