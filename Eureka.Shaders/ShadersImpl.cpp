
#include <ColoredVertexVS.spvhpp>
#include <ColoredVertexFS.spvhpp>
#include <TriangleVS.spvhpp>
#include <TriangleFS.spvhpp>


#define EUREKA_DEFINE_SHADER_ID(IDENTIFIER, SHADER_FULL_NAME) \
extern const eureka::ShaderId IDENTIFIER; \
const eureka::ShaderId IDENTIFIER{ SHADER_FULL_NAME, sizeof(SHADER_FULL_NAME) }; 
//static_assert(false);

#undef EUREKA_SHADER_IDENTIFIER
#define EUREKA_SHADER_IDENTIFIER(IDENTIFIER) \
EUREKA_DEFINE_SHADER_ID(IDENTIFIER, EUREKA_CONCAT(SHADER_BYTES_, IDENTIFIER));

#define EUREKA_SHADER_IMPL
#undef EUREKA_SHADERS_HEADER
#include "ShadersDeclare.hpp"