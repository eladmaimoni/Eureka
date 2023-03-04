#pragma once
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/ShaderModule.hpp"
#include "ShadersDeclare.hpp"
#include <filesystem>

namespace std
{
    template <>
    struct hash<eureka::vulkan::ShaderId>
    {
        std::size_t operator()(const eureka::vulkan::ShaderId& s) const
        {
            return std::hash<void*>()((void*)s.ptr);
        }
    };
} // namespace std

namespace eureka::vulkan
{

    class ShaderCache
    {
        std::shared_ptr<Device> _device;

    public:
        ShaderCache() = default;
        ShaderCache(std::shared_ptr<Device> device);
        ShaderCache(ShaderCache&& that) = default;
        ShaderCache& operator=(ShaderCache&& rhs) = default;
        ~ShaderCache();
        ShaderModule LoadShaderModule(const ShaderId& id) const;
        ShaderModule LoadShaderModule(const std::filesystem::path& filename) const;
    };

} // namespace eureka::vulkan
