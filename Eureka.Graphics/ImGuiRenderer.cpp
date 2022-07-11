#include "ImGuiRenderer.hpp"

namespace eureka
{


    void ImGuiRenderer::HandleResize(uint32_t w, uint32_t h) const
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }

}

