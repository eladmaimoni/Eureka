#include "ShadersCache.hpp"
#include <Shader1.spvhpp>
#include <cstdint>

 
namespace eureka
{  

    const uint8_t* GetShader()
    { 
        return nullptr;
    }

    ShaderCache::ShaderCache(std::shared_ptr<vk::raii::Device> device) 
        :
        _device(std::move(device))
    {

        vk::ShaderModuleCreateInfo createInfo
        {
            .flags = vk::ShaderModuleCreateFlags(),
            .codeSize = ShadedMeshVS.size,
            .pCode = reinterpret_cast<const uint32_t*>(ShadedMeshVS.ptr)
        };
        _shaders.emplace(ShadedMeshVS, _device->createShaderModule(createInfo));
    }

    ShaderCache::~ShaderCache()
    {

    }

}
