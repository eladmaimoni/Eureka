#include "ImGuiViewPass.hpp"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include <profiling.hpp>
#include <imgui_internal.h>
#include "AsyncDataLoader.hpp"
#include "../Eureka.Vulkan/Pipeline.hpp"
#include "../Eureka.Vulkan/PipelinePresets.hpp"


namespace eureka::graphics
{

    //static_assert(sizeof(ImDrawVert) == sizeof(ImGuiVertex));
    //static_assert(offsetof(ImDrawVert, pos) == offsetof(ImGuiVertex, position));
    //static_assert(offsetof(ImDrawVert, uv) == offsetof(ImGuiVertex, uv));
    //static_assert(offsetof(ImDrawVert, col) == offsetof(ImGuiVertex, color));

    inline constexpr uint64_t EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES = 4 * 1024 * 1024;



    ImGuiViewPass::ImGuiViewPass(
        GlobalInheritedData globalInheritedData,
        std::shared_ptr<IImGuiLayout> layout
    ) :
        IViewPass(std::move(globalInheritedData)),
        _layout(std::move(layout)),
        _descriptorSet(_globalInheritedData.device, _globalInheritedData.descriptor_allocator),
        _fontImage(_globalInheritedData.device, _globalInheritedData.resource_allocator)
    {

    }

    ImGuiViewPass::~ImGuiViewPass()
    {
        _layout->OnDeactivated();
        ImGui_ImplGlfw_Shutdown();
    }



    future_t<void> ImGuiViewPass::Setup()
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("imgui setup", eureka::profiling::Color::Red, eureka::profiling::PROFILING_CATEGORY_INIT);
        //co_await concurrencpp::resume_on(*_poolExecutor);


        //
        // check if if we have an imgui.ini file
        // 
        ImguiLayoutProps props{};
        if (std::filesystem::exists("imgui.ini"))
        {
            props.has_ini_file = true;
        }
        
        
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

        //
        // vulkan stuff
        //
        vulkan::PipelineLayoutCreationPreset imguiPipelineLayoutPreset(vulkan::PipelinePresetType::eImGui, *_globalInheritedData.layout_cache);
        _pipelineLayout = std::make_shared<vulkan::PipelineLayout>(_globalInheritedData.device, imguiPipelineLayoutPreset.GetCreateInfo());
        vulkan::PipelineCreationPreset imguiPipelinePreset(
            vulkan::PipelinePresetType::eImGui, *_globalInheritedData.shader_cache, _pipelineLayout->Get(), _targetInheritedData.render_pass->Get());
        
        _pipeline = vulkan::Pipeline(_globalInheritedData.device, _pipelineLayout, _targetInheritedData.render_pass, imguiPipelinePreset.GetCreateInfo());

        _vertexIndexBuffer = vulkan::HostVisibleVertexAndIndexTransferableDeviceBuffer(_globalInheritedData.resource_allocator, EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES);


        // Create font texture
        unsigned char* fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

        _fontImage = vulkan::Image2D(
            _globalInheritedData.device,
            _globalInheritedData.resource_allocator,
            vulkan::Image2DProperties
            {
                VkExtent2D{.width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight) },
                vulkan::Image2DAllocationPreset::eR8G8B8A8UnormSampledShaderResource
            }
        );

        _fontSampler = vulkan::CreateSampler(_globalInheritedData.device, vulkan::SamplerCreationPreset::eLinearClampToEdge);

        vulkan::ImageStageUploadDesc transferDesc
        {
            .unpinned_src_span = std::span(fontData, uploadSize),
            .stage_zone_offset = 0,
            .destination_image = _fontImage.Get(),
            .destination_image_extent = VkExtent3D{.width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight), .depth = 1}
        };
     

        _descriptorSet = vulkan::FreeableDescriptorSet(
            _globalInheritedData.device,
            _globalInheritedData.descriptor_allocator,
            _globalInheritedData.layout_cache->GetLayoutHandle(vulkan::DescriptorSet0PresetType::eSingleTexture)
        );

        std::array<VkDescriptorImageInfo, 1> imageInfo
        {
            VkDescriptorImageInfo
            {
                .sampler = _fontSampler.Get(),
                .imageView = _fontImage.GetView(),
                .imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        };

        _descriptorSet.SetBindings(
            0, 
            VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            imageInfo
        );


        co_await _globalInheritedData.async_data_loader->UploadImageAsync(transferDesc);

  
        _initialized = true;
        _active = _initialized && _validSize;
        _layout->OnActivated(props);

        co_return;
    }

    void ImGuiViewPass::Layout()
    {
        if (!_active) return;

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        
        _layout->UpdateLayout();


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
            //DEBUGGER_TRACE("IMGUI rendering failed, vertex buffer not large enough. required size = {} available size = {}", totalSize, _vertexIndexBuffer.ByteSize());
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

    void ImGuiViewPass::RecordDrawCommands(vulkan::LinearCommandBufferHandle commandBuffer)
    {
        if (!_active) return;
        ImDrawData* imDrawData = ImGui::GetDrawData();
        if (!imDrawData) return;

        ImGuiIO& io = ImGui::GetIO();




        auto ownerViewpot = imDrawData->OwnerViewport;

      
        VkViewport viewport
        {
            .x = ownerViewpot->Pos.x,
            .y = ownerViewpot->Pos.y,
            .width = ownerViewpot->Size.x,
            .height = ownerViewpot->Size.y,
            .minDepth = 0.0f, 
            .maxDepth = 1.0f
        };


        if (viewport.width <= 0.0f || viewport.height <= 0.0f) return;

        commandBuffer.Bind(
            VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
            _pipelineLayout->Get(),
            _descriptorSet.Get(),
            0u
        );
        commandBuffer.BindGraphicsPipeline(_pipeline.Get());
        commandBuffer.SetViewport(viewport);
        vulkan::ScaleTranslatePushConstantsBlock pushConstanst
        {
            .scale = Eigen::Vector2f(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y),
            .translate = Eigen::Vector2f(-1.0f, -1.0f)
        };

        commandBuffer.PushConstants(_pipelineLayout->Get(), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, pushConstanst);


        // Render commands

        int32_t globalVertexOffset = 0;
        int32_t globalIndexOffset = 0;

        if (imDrawData->CmdListsCount > 0)
        {
            commandBuffer.BindIndexBuffer(
                _vertexIndexBuffer.Buffer(),
                VkIndexType::VK_INDEX_TYPE_UINT16
            );

            commandBuffer.BindVertexBuffer(
                 _vertexIndexBuffer.Buffer(),
                 _vertexBufferOffset
            );

            for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
            {
                const ImDrawList* imDrawList = imDrawData->CmdLists[i];
                for (auto j = 0; j < imDrawList->CmdBuffer.Size; ++j)
                {
                    const ImDrawCmd* imDrawCommand = &imDrawList->CmdBuffer[j];
                    VkRect2D scissorRect;
                    scissorRect.offset.x = std::max((int32_t)(imDrawCommand->ClipRect.x), 0);
                    scissorRect.offset.y = std::max((int32_t)(imDrawCommand->ClipRect.y), 0);
                    scissorRect.extent.width = (uint32_t)(imDrawCommand->ClipRect.z - imDrawCommand->ClipRect.x);
                    scissorRect.extent.height = (uint32_t)(imDrawCommand->ClipRect.w - imDrawCommand->ClipRect.y);

                    
                    commandBuffer.SetScissor(scissorRect);
                    commandBuffer.DrawIndexed(
                        imDrawCommand->ElemCount, 
                        1, 
                        globalIndexOffset + imDrawCommand->IdxOffset, 
                        globalVertexOffset + imDrawCommand->VtxOffset,
                        1
                    );
                }
                globalIndexOffset += imDrawList->IdxBuffer.Size;
                globalVertexOffset += imDrawList->VtxBuffer.Size;
            }
        }
    }

    void ImGuiViewPass::BindToTargetPass(TargetInheritedData inheritedData)
    {
        _targetInheritedData = std::move(inheritedData);

        Setup();
    }

    void ImGuiViewPass::HandleResize(uint32_t w, uint32_t h)
    {
        _validSize = (w > 0 && h > 0);
        _active = _initialized && _validSize;

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);


    }


}

