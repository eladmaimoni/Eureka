cmake_minimum_required (VERSION 3.21)
find_package(Catch2 CONFIG REQUIRED)

add_executable(
    Eureka.UnitTests
    "main.cpp"
    "tests.cpp"

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
    Catch2::Catch2 
    Catch2::Catch2WithMain
    Eigen3::Eigen
    Vulkan::Vulkan
    glfw
)
#Magnum::GLContext Magnum::Application