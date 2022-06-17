
#ifndef EUREKA_SHADER_ID
#define EUREKA_SHADER_ID 
namespace eureka
{
    struct ShaderId
    {
        const unsigned char* ptr;
        const unsigned long long size;
        vk::ShaderStageFlagBits shader_type;

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
extern const eureka::ShaderId IDENTIFIER;
#endif
#endif

//////////////////////////////////////////////////////////////////////////
//                  Shaders Deceleration
//////////////////////////////////////////////////////////////////////////
EUREKA_SHADER_IDENTIFIER(ColoredVertexVS, vk::ShaderStageFlagBits::eVertex);
EUREKA_SHADER_IDENTIFIER(ColoredVertexFS, vk::ShaderStageFlagBits::eFragment);
EUREKA_SHADER_IDENTIFIER(ShadedMeshWithNormalMapVS, vk::ShaderStageFlagBits::eVertex);
EUREKA_SHADER_IDENTIFIER(PhongShadedMeshWithNormalMapFS, vk::ShaderStageFlagBits::eFragment);
EUREKA_SHADER_IDENTIFIER(TriangleVS, vk::ShaderStageFlagBits::eVertex);
EUREKA_SHADER_IDENTIFIER(TriangleFS, vk::ShaderStageFlagBits::eFragment);
EUREKA_SHADER_IDENTIFIER(ImGuiVS, vk::ShaderStageFlagBits::eVertex);
EUREKA_SHADER_IDENTIFIER(ImGuiFS, vk::ShaderStageFlagBits::eFragment);
EUREKA_SHADER_IDENTIFIER(ImGuiGLSLVS, vk::ShaderStageFlagBits::eVertex);
EUREKA_SHADER_IDENTIFIER(ImGuiGLSLFS, vk::ShaderStageFlagBits::eFragment);

#endif // SHADERS_HEADER