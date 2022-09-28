#pragma once
#include <imgui.h>

typedef struct GLFWwindow GLFWwindow;

namespace eureka::graphics
{
	class ImGuiIntegration
	{
	public:
		ImGuiIntegration();
		~ImGuiIntegration();
		void BindToGLFWWindow(GLFWwindow* window);
	};

}

