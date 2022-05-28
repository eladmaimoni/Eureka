#include <macros.hpp>

namespace eureka
{
    struct ShaderId
    {
        const unsigned char* ptr;
        const unsigned long long size;

        bool operator==(const ShaderId& s) const
        {
            return ptr == s.ptr && this->size == s.size;
        }
    };
}

#ifndef EUREKA_SHADERS_HEADER
#define EUREKA_SHADERS_HEADER

#ifndef EUREKA_SHADER_IMPL
#ifndef EUREKA_SHADER_IDENTIFIER
#define EUREKA_SHADER_IDENTIFIER(IDENTIFIER) \
extern const unsigned char EUREKA_CONCAT(SHADER_BYTES_, IDENTIFIER)[];\
extern const eureka::ShaderId IDENTIFIER;
#endif
#endif

//////////////////////////////////////////////////////////////////////////
//                  Shaders Deceleration
//////////////////////////////////////////////////////////////////////////
EUREKA_SHADER_IDENTIFIER(ShadedMeshVS);

#endif // SHADERS_HEADER