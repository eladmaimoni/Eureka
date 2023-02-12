#include "ShaderModule.hpp"
#include <move.hpp>

namespace eureka::vulkan
{
    ShaderModule::ShaderModule(
        std::shared_ptr<Device> device, 
        const VkShaderModuleCreateInfo& shaderModuleCreateInfo
    ) :
        _device(std::move(device))
    {
        _shaderModule = _device->CreateShaderModule(shaderModuleCreateInfo);
    }

    ShaderModule::ShaderModule(ShaderModule&& that) noexcept
        :
        _device(std::move(that._device)),
        _shaderModule(steal(that._shaderModule))
    {

    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& rhs) noexcept
    {
        _device = std::move(rhs._device);
        _shaderModule = steal(rhs._shaderModule);
        return *this;
    }
   
    ShaderModule::~ShaderModule()
    {
        if (_shaderModule)
        {
            _device->DestroyShaderModule(_shaderModule);
        }
    }
}

