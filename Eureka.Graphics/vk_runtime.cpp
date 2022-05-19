#include "vk_runtime.hpp"

/*
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code

*/
namespace eureka
{


    void ValidateRequiredExtentionsExists(const VkRuntimeDesc& desc)
    {
        auto supported_extentions = vk::enumerateInstanceExtensionProperties();

        for (auto required_extention : desc.required_instance_extentions)
        {
            bool found = false;
            for (auto supported_extention : supported_extentions)
            {
                std::string_view supported_extention_name(supported_extention.extensionName.data());
                if (supported_extention_name == required_extention)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw vk::SystemError(vk::Result::eErrorExtensionNotPresent);
            }
        }
    }

    void ValidateRequiredLayersExists(const VkRuntimeDesc& desc)
    {
        auto supported_layers = vk::enumerateInstanceLayerProperties();

        for (auto required_layer : desc.required_layers)
        {
            bool found = false;
            for (auto supported_layer : supported_layers)
            {
                std::string_view supported_layer_name(supported_layer.layerName.data());
                if (supported_layer_name == required_layer)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw vk::SystemError(vk::Result::eErrorLayerNotPresent);
            }
        }
    }

    void VkRuntime::InitInstance(const VkRuntimeDesc& desc)
    {
        uint32_t version;
        VK_CHECK(vk::enumerateInstanceVersion(&version));

        DEBUGGER_TRACE("vulkan api version {}.{}.{}",
            VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version),
            VK_API_VERSION_PATCH(version)
        );

        auto appInfo = vk::ApplicationInfo(
            "Eureka",
            version,
            "Eureka Engine",
            version,
            version
        );

        ValidateRequiredExtentionsExists(desc);
        ValidateRequiredLayersExists(desc);
        

        auto createInfo = vk::InstanceCreateInfo(
            vk::InstanceCreateFlags(),
            &appInfo,
            static_cast<uint32_t>(desc.required_layers.size()), desc.required_layers.data(), // enabled layers
            static_cast<uint32_t>(desc.required_instance_extentions.size()),
            desc.required_instance_extentions.data()
        );

        _instance = vk::createInstance(createInfo);
    }



}