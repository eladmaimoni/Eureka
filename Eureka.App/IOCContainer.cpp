#include "IOCContainer.hpp"
#include "VkHelpers.hpp"
#include "../Eureka.Graphics/RenderingSystem.hpp"

namespace eureka
{
    InstanceConfig CreateInstanceConfig(const GLFWRuntime& glfw)
    {
        InstanceConfig runtime_desc{};


        runtime_desc.required_instance_extentions = glfw.QueryRequiredVulkanExtentions();
        runtime_desc.required_instance_extentions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        runtime_desc.required_layers.emplace_back(eureka::VK_LAYER_VALIDATION);

        DEBUGGER_TRACE("Requested instance extentions = {}", runtime_desc.required_instance_extentions);
        DEBUGGER_TRACE("Requested instance layers = {}", runtime_desc.required_layers);

        return runtime_desc;
    }

    DeviceContextConfig CreateDeviceContextConfig(vk::SurfaceKHR presentationSurface = nullptr)
    {
        DeviceContextConfig device_context_desc{};
        device_context_desc.presentation_surface = presentationSurface;
        device_context_desc.required_layers.emplace_back(VK_LAYER_VALIDATION);
        device_context_desc.required_extentions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        DEBUGGER_TRACE("Requested device extentions = {}", device_context_desc.required_extentions);
        DEBUGGER_TRACE("Requested device layers = {}", device_context_desc.required_layers);
        return device_context_desc;
    }

    IOCContainer::IOCContainer()
        : 
        _instance(CreateInstanceConfig(_glfw))
    {
        _deviceContext.Init(_instance, CreateDeviceContextConfig());


    }

    IOCContainer::~IOCContainer()
    {
       
    }

    std::unique_ptr<RenderingSystem> IOCContainer::CreateRenderingSystem() 
    {
        return std::make_unique<RenderingSystem>(_instance, _deviceContext, _glfw);
    }

}