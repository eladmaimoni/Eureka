cmake_minimum_required (VERSION 3.21)

# Add source to this project's executable.
add_library(
    Eureka.Graphics 
    "vk_runtime.hpp"
    "vk_runtime.cpp"
    "vk_error_handling.hpp"
    "vk_error_handling.cpp"
)
set_compiler_flags(Eureka.Graphics)

target_include_directories(
    Eureka.Graphics
    PRIVATE 
    ${VULKAN_HPP_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.Graphics 
    PRIVATE 
    Eureka.Core 
    Eigen3::Eigen
)

