#pragma once
#include "Device.hpp"
#include <containers_aliases.hpp>

namespace eureka::vulkan
{
    class ShaderModule
    {
        std::shared_ptr<Device> _device{ nullptr };
        VkShaderModule          _shaderModule{ nullptr };
    public:
        ShaderModule() = default;
        ShaderModule(std::shared_ptr<Device> device, const VkShaderModuleCreateInfo& shaderModuleCreateInfo);
        ~ShaderModule();
        ShaderModule(ShaderModule&& that) noexcept;
        ShaderModule& operator=(ShaderModule&& rhs) noexcept;
        ShaderModule(const ShaderModule&) = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;
        VkShaderModule Get() const { return _shaderModule; }
    };

    struct ShadersPipeline
    {
        svec2<ShaderModule>                    modules;
        svec2<VkPipelineShaderStageCreateInfo> stages;
    };
}

