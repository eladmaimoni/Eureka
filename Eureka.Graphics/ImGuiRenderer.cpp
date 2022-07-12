#include "ImGuiRenderer.hpp"

namespace eureka
{


    ImGuiRenderer::ImGuiRenderer(DeviceContext& deviceContext, std::shared_ptr<PipelineCache> pipelineCache, std::shared_ptr<MTDescriptorAllocator> descPool, std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext, std::shared_ptr<OneShotCopySubmissionHandler> oneShotCopySubmissionHandler, std::shared_ptr<HostWriteCombinedRingPool> uploadPool, PoolExecutor poolExecutor) :
        _deviceContext(deviceContext),
        _uploadPool(std::move(uploadPool)),
        _poolExecutor(std::move(poolExecutor)),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _oneShotCopySubmissionHandler(std::move(oneShotCopySubmissionHandler))
    {
        Setup(std::move(pipelineCache), std::move(descPool));
    }

    future_t<void> ImGuiRenderer::Setup(std::shared_ptr<PipelineCache> pipelineCache, std::shared_ptr<MTDescriptorAllocator> descPool)
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("imgui setup", Profiling::Color::Red, Profiling::PROFILING_CATEGORY_INIT);
        co_await concurrencpp::resume_on(*_poolExecutor);


        _vertexIndexBuffer = VertexAndIndexHostVisibleDeviceBuffer(_deviceContext.Allocator(), BufferConfig{ .byte_size = EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES });

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

        _fontImage = SampledImage2D(_deviceContext, fontImageProps);

        ImageStageUploadDesc transferDesc
        {
            .unpinned_src_span = std::span(fontData, uploadSize),
            .stage_zone_offset = 0,
            .destination_image = _fontImage.Get(),
            .destination_image_extent = vk::Extent3D{.width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight), .depth = 1}
        };

        auto stageBuffer = co_await _uploadPool->EnqueueAllocation(uploadSize);


        stageBuffer.Assign(transferDesc.unpinned_src_span, 0);

        auto [preTransferBarrier, bufferImageCopy, copyRelease, graphicsAcquire] =
            MakeCopyQueueSampledImageUpload(
                _submissionThreadExecutionContext->CopyQueue(),
                _submissionThreadExecutionContext->GraphicsQueue(),
                transferDesc
            );



        _pipeline = pipelineCache->GetImGuiPipeline();

        _descriptorSet = descPool->AllocateSet(_pipeline->GetFragmentShaderDescriptorSetLayout());

        std::array<vk::DescriptorImageInfo, 1> imageInfo
        {
            vk::DescriptorImageInfo
            {
                .sampler = _fontImage.GetSampler(),
                .imageView = _fontImage.GetView(),
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
            }
        };


        _descriptorSet.SetBindings(0, vk::DescriptorType::eCombinedImageSampler, imageInfo);

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
                { copyRelease }
            );

            
        }

        auto fut = _oneShotCopySubmissionHandler->AppendOneShotCommandBufferSubmission(std::move(uploadCommandBuffer));

        // record graphics queue acquire


        co_await fut;

        co_await concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());

        _active = true;

        co_return;
    }

    void ImGuiRenderer::Layout()
    {
        if (!_active) return;

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
    }

    void ImGuiRenderer::SyncBuffers()
    {
        PROFILE_CATEGORIZED_SCOPE("imgui vertex update", Profiling::Color::Brown, Profiling::PROFILING_CATEGORY_RENDERING);

        ImDrawData* imDrawData = ImGui::GetDrawData();

        if (!imDrawData)
        {
            return;
        }
        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        auto totalSize = vertexBufferSize + indexBufferSize;

        if (totalSize > _vertexIndexBuffer.ByteSize())
        {
            DEBUGGER_TRACE("IMGUI rendering failed, vertex buffer not large enough. required size = {} available size = {}", totalSize, _vertexIndexBuffer.ByteSize());
            return;
        }

        if ((vertexBufferSize == 0) || (indexBufferSize == 0))
        {
            return;
        }

        auto deviceMappedPtr = _vertexIndexBuffer.Ptr<uint8_t>();

        _vertexBufferOffset = 0;

        for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            auto bytes = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
            std::memcpy(deviceMappedPtr, cmd_list->IdxBuffer.Data, bytes);
            deviceMappedPtr += bytes;
            _vertexBufferOffset += bytes;
        }

        auto alignmentRemainder = _vertexBufferOffset % sizeof(ImDrawVert);
        deviceMappedPtr += alignmentRemainder;
        _vertexBufferOffset += alignmentRemainder;

        for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            auto bytes = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            std::memcpy(deviceMappedPtr, cmd_list->VtxBuffer.Data, bytes);
            deviceMappedPtr += bytes;
        }
    }

    void ImGuiRenderer::RecordDrawCommands(vk::CommandBuffer commandBuffer)
    {
        if (!_active) return;

        ImGuiIO& io = ImGui::GetIO();


        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            _pipeline->Layout(),
            0,
            { _descriptorSet.Get() },
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            _pipeline->Get()
        );

        ImGuiPushConstantsBlock pushConstanst
        {
            .scale = Eigen::Vector2f(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y),
            .translate = Eigen::Vector2f(-1.0f, -1.0f)
        };

        commandBuffer.pushConstants<ImGuiPushConstantsBlock>(_pipeline->Layout(), vk::ShaderStageFlagBits::eVertex, 0, { pushConstanst });


        // Render commands
        ImDrawData* imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0)
        {

            //VkDeviceSize offsets[1] = { 0 };

            commandBuffer.bindIndexBuffer(
                _vertexIndexBuffer.Buffer(),
                0,
                vk::IndexType::eUint16
            );

            commandBuffer.bindVertexBuffers(
                0,
                { _vertexIndexBuffer.Buffer() },
                { _vertexBufferOffset }
            );

            //vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
            //vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
            {
                const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                for (auto j = 0; j < cmd_list->CmdBuffer.Size; ++j)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                    vk::Rect2D scissorRect;
                    scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                    scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                    scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                    scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);

                    commandBuffer.setScissor(0, { scissorRect });

                    commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 1);
                    //vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

                    //vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                    indexOffset += pcmd->ElemCount;
                }
                vertexOffset += cmd_list->VtxBuffer.Size;
            }
        }
    }

    void ImGuiRenderer::HandleResize(uint32_t w, uint32_t h) const
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }

}

