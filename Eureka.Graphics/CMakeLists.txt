

set_source_group(
    core 
    "VkHelpers.hpp"
    "vk_error_handling.hpp"
    "vk_error_handling.cpp"
)

set_source_group(
    infrastructure 
    "VkMemAllocator.cpp"
    "GraphicsDefaults.hpp"
)

set_source_group(
    wrappers 
    "PipelineTypes.hpp"
    "Mesh.hpp"
    "Mesh.cpp"
    "Instance.hpp" 
    "Instance.cpp"
    "CommandBuffer.hpp" 
    "CommandBuffer.cpp" 
    "SwapChain.hpp" 
    "SwapChain.cpp" 
    "Pipeline.hpp" 
    "Pipeline.cpp"
    "DeviceContext.hpp" 
    "DeviceContext.cpp"
    "Image.hpp"
    "Image.cpp"
    "RenderPass.hpp"
    "RenderPass.cpp"
    "RenderTarget.hpp"
    "RenderTarget.cpp"
    "Buffer.hpp"
    "Buffer.cpp"
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

set_source_group(
    objects 
     "Camera.hpp"
     "Camera.cpp"      
     "UpdateQueue.hpp"
     "UpdateQueue.cpp" 
)


add_library(
    Eureka.Graphics
    ${core}
    ${infrastructure}
    ${rendering}
    ${wrappers}
    ${windowing}
    ${objects}
)


set_compiler_flags(Eureka.Graphics)

target_precompile_headers(
    Eureka.Graphics 
    REUSE_FROM
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
    Eureka.Shaders 
    glfw

    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    
)

