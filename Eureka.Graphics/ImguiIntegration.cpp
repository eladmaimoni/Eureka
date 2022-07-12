#include "ImguiIntegration.hpp"

namespace eureka
{


    ImGuiIntegration::ImGuiIntegration() 
    {
        ImGui::CreateContext();
    }


    ImGuiIntegration::~ImGuiIntegration()
    {
        ImGui::DestroyContext();
    }

}


