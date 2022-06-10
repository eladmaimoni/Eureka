
#include "ShadersDeclare.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace std 
{

    template <>
    struct hash<eureka::ShaderId>
    {
        std::size_t operator()(const eureka::ShaderId& s) const
        {
            return std::hash<void*>()((void*)s.ptr);
        }
    };

}
namespace eureka
{ 

    class ShaderCache
    {
        std::shared_ptr<vkr::Device> _device;
        //std::unordered_map<ShaderId, vkr::ShaderModule> _shaders;
        vkr::PipelineCache _piplineCache{ nullptr };
    public:
        ShaderCache() = default;
        ShaderCache(std::shared_ptr<vkr::Device> device);
        ShaderCache(ShaderCache&& that) = default;
        ShaderCache& operator=(ShaderCache&& rhs) = default;
        ~ShaderCache();
        vkr::ShaderModule LoadShaderModule(const ShaderId& id) const;
        vkr::ShaderModule LoadShaderModule(const std::filesystem::path& filename) const;
        const vkr::PipelineCache& Cache() const { return _piplineCache; }
    };

    
}

