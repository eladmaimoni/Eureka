


add_executable(
    ZZZ_SAMPLE
    "stb_image.h"
    "stb_image.cpp"
    "zzz_sample.cpp"
) 

set_compiler_flags(ZZZ_SAMPLE)  


target_shared_precompiled_headers(
    ZZZ_SAMPLE
    Eureka.Precompiled
)

target_link_libraries(
    ZZZ_SAMPLE
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
    glm::glm
)