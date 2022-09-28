#include "ShadersCache.hpp"
#include <cstdint>
#include <debugger_trace.hpp>
#include <fstream>

namespace eureka::vulkan
{  
    ShaderCache::ShaderCache(std::shared_ptr<Device> device) 
        :
        _device(std::move(device))
    {
    
    }

    ShaderModule ShaderCache::LoadShaderModule(const ShaderId& id) const 
    {
        VkShaderModuleCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .flags = {},
            .codeSize = id.size,
            .pCode = reinterpret_cast<const uint32_t*>(id.ptr)
        };

        return ShaderModule(_device, createInfo);
    }

    ShaderModule ShaderCache::LoadShaderModule(const std::filesystem::path& filename) const
    {
        if (!std::filesystem::exists(filename))
        {
            throw std::invalid_argument("bad path");
        }

        std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

        uint32_t shaderSize = static_cast<uint32_t>(is.tellg());
        is.seekg(0, std::ios::beg);
        // Copy file contents into a buffer
        std::vector<char> shaderCode(shaderSize);
        is.read(shaderCode.data(), shaderSize);
        is.close();
  
        VkShaderModuleCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .flags = {},
            .codeSize = shaderSize,
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
        };

        return ShaderModule(_device, createInfo);
    }

    ShaderCache::~ShaderCache()
    {
        
    }

}
