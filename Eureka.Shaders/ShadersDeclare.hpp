#ifndef EUREKA_SHADER_ID
#define EUREKA_SHADER_ID 

#include <volk.h>
#include <macros.hpp>

namespace eureka::vulkan
{
    struct ShaderId
    {
        const unsigned char* ptr;
        const unsigned long long size;
        VkShaderStageFlagBits shader_type;

        bool operator==(const ShaderId& s) const
        {
            return ptr == s.ptr && size == s.size && shader_type == s.shader_type;
        }
    };
}
#endif

#ifndef EUREKA_SHADERS_HEADER
#define EUREKA_SHADERS_HEADER

#ifndef EUREKA_SHADER_IMPL
#ifndef EUREKA_SHADER_IDENTIFIER
#define EUREKA_SHADER_IDENTIFIER(IDENTIFIER, SHADER_TYPE) \
extern const unsigned char EUREKA_CONCAT(SHADER_BYTES_, IDENTIFIER)[];\
extern const eureka::vulkan::ShaderId IDENTIFIER;
#endif
#endif

//////////////////////////////////////////////////////////////////////////
//                  Shaders Deceleration
//////////////////////////////////////////////////////////////////////////
EUREKA_SHADER_IDENTIFIER(ColoredVertexVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(ColoredVertexFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
EUREKA_SHADER_IDENTIFIER(ShadedMeshWithNormalMapVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(PhongShadedMeshWithNormalMapFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
EUREKA_SHADER_IDENTIFIER(TriangleVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(TriangleFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
EUREKA_SHADER_IDENTIFIER(ImGuiVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(ImGuiFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
EUREKA_SHADER_IDENTIFIER(ImGuiGLSLVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(ImGuiGLSLFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
EUREKA_SHADER_IDENTIFIER(Textured2DRegionVS, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
EUREKA_SHADER_IDENTIFIER(Textured2DRegionFS, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
#endif // SHADERS_HEADER