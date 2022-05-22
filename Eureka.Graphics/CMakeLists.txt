cmake_minimum_required (VERSION 3.21)

# Add source to this project's executable.
add_library(
    Eureka.Graphics 
    "VkRuntime.hpp"
    "VkRuntime.cpp"
    "vk_error_handling.hpp"
    "vk_error_handling.cpp"
    "glfw_runtime.hpp"
    "glfw_runtime.cpp"
    "SwapChainTarget.hpp"
    "SwapChainTarget.cpp"
    "CommandBuffer.hpp"
    "CommandBuffer.cpp"

)

set_compiler_flags(Eureka.Graphics)

target_shared_precompiled_headers(
    Eureka.Graphics 
    Eureka.Precompiled
)



target_include_directories(
    Eureka.Graphics
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE 
    ${VULKAN_HPP_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.Graphics 
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core 
    Eigen3::Eigen
    Vulkan::Vulkan
    glfw
)

