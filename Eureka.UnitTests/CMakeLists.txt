

add_executable(
    Eureka.UnitTests
    "main.cpp"
    "tests.cpp"
    "formatting.tests.cpp"
) 
set_compiler_flags(Eureka.UnitTests)   

target_shared_precompiled_headers(
    Eureka.UnitTests 
    Eureka.Precompiled
)

target_link_libraries(
    Eureka.UnitTests
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
    Catch2::Catch2 
    Catch2::Catch2WithMain
    glfw
)
#Magnum::GLContext Magnum::Application