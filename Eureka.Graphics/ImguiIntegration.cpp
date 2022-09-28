#include "ImguiIntegration.hpp"
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>

namespace eureka::graphics
{
    ImGuiIntegration::ImGuiIntegration() 
    {
        ImGui::CreateContext();
    }


    ImGuiIntegration::~ImGuiIntegration()
    {
        ImGui::DestroyContext();
    }

    void ImGuiIntegration::BindToGLFWWindow(GLFWwindow* window)
    {
        if (!ImGui_ImplGlfw_InitForVulkan(window, true))
        {
            throw std::runtime_error("bad");
        }
    }

}


