#pragma once
#include <imgui.h>
#include <Buffer.hpp>
#include <imgui_internal.h>

namespace eureka
{
    
    inline constexpr uint64_t EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES = 1024 * 1024;

    class ImGuiRenderer
    {
        VertexAndIndexHostVisibleDeviceBuffer _vertexIndexBuffer;

        void Layout()
        {
            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
            ImGui::Render();
        }

        ImGuiRenderer(DeviceContext& deviceContext)
            : _vertexIndexBuffer(deviceContext.Allocator(), BufferConfig{.byte_size = EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES })
        {

        }

        void SyncBuffers()
        {
            PROFILE_CATEGORIZED_SCOPE("imgui vertex update", Profiling::Color::Brown, Profiling::PROFILING_CATEGORY_RENDERING);
            ImDrawData* imDrawData = ImGui::GetDrawData();

            VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
            VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

            if ((vertexBufferSize == 0) || (indexBufferSize == 0)) 
            {
                return;
            }

            auto deviceMappedPtr = _vertexIndexBuffer.Ptr<uint8_t>();

            for (auto i = 0; i < imDrawData->CmdListsCount; ++i) 
            {
                const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                auto bytes = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
                std::memcpy(deviceMappedPtr, cmd_list->IdxBuffer.Data, bytes);
                deviceMappedPtr += bytes;
            }

            for (auto i = 0; i < imDrawData->CmdListsCount; ++i)
            {
                const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                auto bytes = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
                std::memcpy(deviceMappedPtr, cmd_list->VtxBuffer.Data, bytes);
                deviceMappedPtr += bytes;
            }
        }
    };

}

