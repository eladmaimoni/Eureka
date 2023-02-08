#include <cstdint>
#include <Textured2DRegionVS.spvhpp>
#include <Textured2DRegionFS.spvhpp>
#include <ImGuiVS.spvhpp>
#include <ImGuiFS.spvhpp>
#include <ImGuiGLSLVS.spvhpp>
#include <ImGuiGLSLFS.spvhpp>
#include <ColoredVertexVS.spvhpp>
#include <ColoredVertexFS.spvhpp>
#include <TriangleVS.spvhpp>
#include <TriangleFS.spvhpp>
#include <ShadedMeshWithNormalMapVS.spvhpp>
#include <PhongShadedMeshWithNormalMapFS.spvhpp>

#define EUREKA_DEFINE_SHADER_ID(IDENTIFIER, SHADER_FULL_NAME, SHADER_TYPE) \
extern const eureka::vulkan::ShaderId IDENTIFIER; \
const eureka::vulkan::ShaderId IDENTIFIER{ SHADER_FULL_NAME, sizeof(SHADER_FULL_NAME), SHADER_TYPE }; 
//static_assert(false);

#undef EUREKA_SHADER_IDENTIFIER
#define EUREKA_SHADER_IDENTIFIER(IDENTIFIER, SHADER_TYPE) \
EUREKA_DEFINE_SHADER_ID(IDENTIFIER, EUREKA_CONCAT(SHADER_BYTES_, IDENTIFIER), SHADER_TYPE);

#define EUREKA_SHADER_IMPL
#undef EUREKA_SHADERS_HEADER
#include "ShadersDeclare.hpp"