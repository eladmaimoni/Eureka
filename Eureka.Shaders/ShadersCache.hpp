
#include "ShadersDeclare.hpp"

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
        std::shared_ptr<vk::raii::Device> _device;
        std::unordered_map<ShaderId, vk::raii::ShaderModule> _shaders;
    public:
        ShaderCache(std::shared_ptr<vk::raii::Device> device);
        ~ShaderCache();

    };

    const uint8_t* GetShader(); 
}

