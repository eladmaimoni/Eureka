#include "ImGuiViewPass.hpp"
#include "imgui_impl_glfw.h"
#include <profiling.hpp>

namespace eureka
{

    static_assert(sizeof(ImDrawVert) == sizeof(ImGuiVertex));
    static_assert(offsetof(ImDrawVert, pos) == offsetof(ImGuiVertex, position));
    static_assert(offsetof(ImDrawVert, uv) == offsetof(ImGuiVertex, uv));
    static_assert(offsetof(ImDrawVert, col) == offsetof(ImGuiVertex, color));

    inline constexpr uint64_t EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES = 4 * 1024 * 1024;



    ImGuiViewPass::ImGuiViewPass(
        DeviceContext& deviceContext, 
        std::shared_ptr<Window> window,
        std::shared_ptr<PipelineCache>  pipelineCache,
        std::shared_ptr<MTDescriptorAllocator> descPool, // TODO move to device
        std::shared_ptr<AsyncDataLoader> asyncDataLoader,
        PoolExecutor poolExecutor // TODO global?
    ) :
        _deviceContext(deviceContext),
        _descPool(std::move(descPool)),
        _poolExecutor(std::move(poolExecutor)),
        _asyncDataLoader(std::move(asyncDataLoader))
    {
        Setup(std::move(window), std::move(pipelineCache));
    }

    ImGuiViewPass::~ImGuiViewPass()
    {
        ImGui_ImplGlfw_Shutdown();
    }



    future_t<void> ImGuiViewPass::Setup(std::shared_ptr<Window> window, std::shared_ptr<PipelineCache> pipelineCache)
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("imgui setup", eureka::profiling::Color::Red, eureka::profiling::PROFILING_CATEGORY_INIT);
        co_await concurrencpp::resume_on(*_poolExecutor);

        //
        // ImGui Stuff
        //
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);


        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        ImGui_ImplGlfw_InitForVulkan(window->WindowHandle(), true);
        //
        // vulkan stuff
        //
        _pipeline = pipelineCache->GetImGuiPipeline();
        _vertexIndexBuffer = VertexAndIndexHostVisibleDeviceBuffer(_deviceContext.Allocator(), BufferConfig{ .byte_size = EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES });


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

        _descriptorSet = _descPool->AllocateSet(_pipeline->GetFragmentShaderDescriptorSetLayout());

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


        co_await _asyncDataLoader->UploadImageAsync(transferDesc);

  
        _active = true;

        co_return;
    }

    void ImGuiViewPass::Layout()
    {
        if (!_active) return;

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        auto vp = ImGui::GetMainViewport();
        /*auto mainDockSpaceId = */ImGui::DockSpaceOverViewport(vp, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("Test Window", nullptr);

        if (_first)
        {
            //auto node = ImGui::DockBuilderGetNode(mainDockSpaceId);
            //node->ChildNodes
            //auto leftDockSpaceId = ImGui::DockBuilderSplitNode(mainDockSpaceId, ImGuiDir_Left, 0.20f, NULL, &mainDockSpaceId);

            //ImGui::DockBuilderDockWindow("Test Window", node->ChildNodes[0]->ID);
            _first = false;
        }
  


        ImGui::Text("Test Text");

        //////////////////////////////////////////////////////////////////////////

        ImGui::Separator();
        ImGui::ShowDemoWindow();
        //if (ImGui::TreeNode("0001: Renderer: Large Mesh Support"))
        //{
        //    ImDrawList* draw_list = ImGui::GetWindowDrawList();
        //    {
        //        static int vtx_count = 60000;
        //        ImGui::SliderInt("VtxCount##1", &vtx_count, 0, 100000);
        //        ImVec2 p = ImGui::GetCursorScreenPos();
        //        for (int n = 0; n < vtx_count / 4; n++)
        //        {
        //            float off_x = (float)(n % 100) * 3.0f;
        //            float off_y = (float)(n % 100) * 1.0f;
        //            ImU32 col = IM_COL32(((n * 17) & 255), ((n * 59) & 255), ((n * 83) & 255), 255);
        //            draw_list->AddRectFilled(ImVec2(p.x + off_x, p.y + off_y), ImVec2(p.x + off_x + 50, p.y + off_y + 50), col);
        //        }
        //        ImGui::Dummy(ImVec2(300 + 50, 100 + 50));
        //        ImGui::Text("VtxBuffer.Size = %d", draw_list->VtxBuffer.Size);
        //    }
        //    {
        //        static int vtx_count = 60000;
        //        ImGui::SliderInt("VtxCount##2", &vtx_count, 0, 100000);
        //        ImVec2 p = ImGui::GetCursorScreenPos();
        //        for (int n = 0; n < vtx_count / (10 * 4); n++)
        //        {
        //            float off_x = (float)(n % 100) * 3.0f;
        //            float off_y = (float)(n % 100) * 1.0f;
        //            ImU32 col = IM_COL32(((n * 17) & 255), ((n * 59) & 255), ((n * 83) & 255), 255);
        //            draw_list->AddText(ImVec2(p.x + off_x, p.y + off_y), col, "ABCDEFGHIJ");
        //        }
        //        ImGui::Dummy(ImVec2(300 + 50, 100 + 20));
        //        ImGui::Text("VtxBuffer.Size = %d", draw_list->VtxBuffer.Size);
        //    }
        //    ImGui::TreePop();
        //}
        //////////////////////////////////////////////////////////////////////////
        ImGui::End();
        ImGui::Render();
    }

    void ImGuiViewPass::SyncBuffers()
    {
        PROFILE_CATEGORIZED_SCOPE("imgui vertex update", eureka::profiling::Color::Brown, eureka::profiling::PROFILING_CATEGORY_RENDERING);

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

        _vertexIndexBuffer.FlushCachesBeforeDeviceRead();
    }

    void ImGuiViewPass::RecordDrawCommands(vk::CommandBuffer commandBuffer)
    {
        if (!_active) return;
        ImDrawData* imDrawData = ImGui::GetDrawData();
        if (!imDrawData) return;

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

        int32_t globalVertexOffset = 0;
        int32_t globalIndexOffset = 0;

        if (imDrawData->CmdListsCount > 0)
        {
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

            for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
            {
                const ImDrawList* imDrawList = imDrawData->CmdLists[i];
                for (auto j = 0; j < imDrawList->CmdBuffer.Size; ++j)
                {
                    const ImDrawCmd* imDrawCommand = &imDrawList->CmdBuffer[j];
                    vk::Rect2D scissorRect;
                    scissorRect.offset.x = std::max((int32_t)(imDrawCommand->ClipRect.x), 0);
                    scissorRect.offset.y = std::max((int32_t)(imDrawCommand->ClipRect.y), 0);
                    scissorRect.extent.width = (uint32_t)(imDrawCommand->ClipRect.z - imDrawCommand->ClipRect.x);
                    scissorRect.extent.height = (uint32_t)(imDrawCommand->ClipRect.w - imDrawCommand->ClipRect.y);

                    commandBuffer.setScissor(0, { scissorRect });
                    commandBuffer.drawIndexed(imDrawCommand->ElemCount, 1, globalIndexOffset + imDrawCommand->IdxOffset, globalVertexOffset + imDrawCommand->VtxOffset, 1);
                }
                globalIndexOffset += imDrawList->IdxBuffer.Size;
                globalVertexOffset += imDrawList->VtxBuffer.Size;
            }
        }
    }

    void ImGuiViewPass::HandleResize(uint32_t w, uint32_t h)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }


}

