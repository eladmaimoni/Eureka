cmake_minimum_required (VERSION 3.21)

set_source_group(
    core 
    "VkHelpers.hpp"
    "vk_error_handling.hpp"
    "vk_error_handling.cpp"
)

set_source_group(
    infrastructure 
    "VkMemAllocator.cpp"
)

set_source_group(
    wrappers 
    "Instance.hpp" 
    "Instance.cpp"
    "CommandBuffer.hpp" 
    "CommandBuffer.cpp" 
    "SwapChainTarget.hpp" 
    "SwapChainTarget.cpp" 
    "Pipeline.hpp" 
    "Pipeline.cpp"
    "DeviceContext.hpp" 
    "DeviceContext.cpp"
    "Images.hpp"
    "Images.cpp"
    "RenderPass.hpp"
    "RenderPass.cpp"
)

set_source_group(
    rendering 
    "RenderingSystem.hpp" 
    "RenderingSystem.cpp" 

)

set_source_group(
    windowing 
    "GLFWRuntime.hpp" 
    "GLFWRuntime.cpp" 

)
add_library(
    Eureka.Graphics
    ${core}
    ${infrastructure}
    ${rendering}
    ${wrappers}
    ${windowing}
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
    glfw
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    
)

