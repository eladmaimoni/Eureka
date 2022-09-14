

set_source_group(
    ui 
    "ImguiIntegration.hpp"
    "ImguiIntegration.cpp"
    "ImGuiViewPass.hpp"
    "ImGuiViewPass.cpp"
    "SceneController.hpp" 
    "SceneController.cpp"
    "imgui_impl_glfw.h"
    "imgui_impl_glfw.cpp"
)
set_source_group(
    renderdoc 
    "RenderDocIntegration.hpp"
    "RenderDocIntegration.cpp"
    "renderdoc_app.h"
)
set_source_group(
    infrastructure 
    "GraphicsDefaults.hpp"
    "UploadRingBuffer.hpp"
    "UploadRingBuffer.cpp"
    "AsyncDataLoader.hpp"
    "AsyncDataLoader.cpp"
    "SecondaryCommandRecorder.hpp"
    "SecondaryCommandRecorder.cpp"
    "SubmissionThreadExecutor.hpp"
    "SubmissionThreadExecutor.cpp"
    "SubmissionThreadExecutionContext.hpp"
    "SubmissionThreadExecutionContext.cpp"
    "FrameContext.hpp"
    "FrameContext.cpp"
    "OneShotCopySubmission.hpp"
    "OneShotCopySubmission.cpp"
    "ShaderReflection.hpp"
    "ShaderReflection.cpp"
)

set_source_group(
    assets
    "AssetLoading.hpp" 
    "AssetLoading.cpp"
)

set_source_group(
    vulkan 
    "VkHelpers.hpp"
    "vk_error_handling.hpp"
    "vk_error_handling.cpp"
    "VkMemAllocator.cpp"
    "Instance.hpp" 
    "Instance.cpp"
    "Commands.hpp" 
    "Commands.cpp"    
    "CommandsUtils.hpp" 
    "CommandsUtils.cpp" 
    "Descriptors.hpp"
    "Descriptors.cpp"
    "DescriptorSetsLayout.hpp"
    "DescriptorSetsLayout.cpp"    
    "SwapChain.hpp" 
    "SwapChain.cpp" 
    "Pipeline.hpp" 
    "Pipeline.cpp"
    "PipelineHelpers.hpp" 
    "PipelineHelpers.cpp"
    "PipelineTypesReflection.hpp"
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
    "Pool.hpp"
    "Pool.cpp"
    "Synchronization.hpp"
    "Synchronization.cpp"
)

set_source_group(
    objects 
    "Mesh.hpp"
    "Mesh.cpp"
    "Model.hpp"
    "Model.cpp" 
    "Camera.hpp"
    "Camera.cpp" 
    "Scene.hpp" 
    "Scene.cpp" 

)

set_source_group(
    rendering 
    "RenderingSystem.hpp" 
    "RenderingSystem.cpp"
    "SceneRenderer.hpp" 
    "SceneRenderer.cpp"
    "IPass.hpp"
    "CameraPass.hpp"
    "CameraPass.cpp"
    "TargetPass.hpp"
    "TargetPass.cpp"
)

set_source_group(
    windowing 
    "GLFWRuntime.hpp" 
    "GLFWRuntime.cpp" 
    "Window.hpp" 
    "Window.cpp" 
)




add_library(
    Eureka.Graphics
    ${assets}
    ${infrastructure}
    ${objects}
    ${rendering}
    ${ui}
    ${vulkan}
    ${windowing}
    ${renderdoc}
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
    ${TINYGLTF_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.Graphics 
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core 
    Eureka.Shaders 
    glfw
    Eureka.Types
    imgui::imgui
    concurrencpp::concurrencpp
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    unofficial::spirv-reflect::spirv-reflect
)

