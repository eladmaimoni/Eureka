#include "ShadersCache.hpp"
#include <cstdint>
#include <debugger_trace.hpp>
 
namespace eureka
{  



    ShaderCache::ShaderCache(std::shared_ptr<vkr::Device> device) 
        :
        _device(std::move(device)),
        _piplineCache(_device->createPipelineCache(vk::PipelineCacheCreateInfo{}))
    {
    
    }

    vkr::ShaderModule ShaderCache::LoadShaderModule(const ShaderId& id) const 
    {
        vk::ShaderModuleCreateInfo createInfo
        {
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = id.size,
            .pCode = reinterpret_cast<const uint32_t*>(id.ptr)
        };
        
        return _device->createShaderModule(createInfo);
    }


    vkr::ShaderModule ShaderCache::LoadShaderModule(const std::filesystem::path& filename) const
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
  
        vk::ShaderModuleCreateInfo createInfo
        {
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = shaderSize,
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
        };

        return _device->createShaderModule(createInfo);
    }

    ShaderCache::~ShaderCache()
    {
        
    }

}
