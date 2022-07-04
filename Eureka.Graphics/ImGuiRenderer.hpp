#pragma once
#include <imgui.h>
#include <Buffer.hpp>
#include <imgui_internal.h>

namespace eureka
{
    

    class ImGuiRenderer
    {

        ImGuiRenderer(DeviceContext& deviceContext)
        {

        }

        void SyncBuffers()
        {
            ImDrawData* imDrawData = ImGui::GetDrawData();

            VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
            VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

            if ((vertexBufferSize == 0) || (indexBufferSize == 0)) 
            {
                return;
            }
        }
    };

}

