#pragma once
#include <imgui.h>
#include <DeviceContext.hpp>

namespace eureka
{
	class ImGuiIntegration
	{
		DeviceContext& _deviceContext;

		ImGuiIntegration(DeviceContext& deviceContext)
			: _deviceContext(deviceContext)
		{
			ImGui::CreateContext();
		}

		~ImGuiIntegration()
		{
			ImGui::DestroyContext();
		}
	};

}

