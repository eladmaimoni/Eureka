

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
    "AsyncDataLoader.hpp"
    "AsyncDataLoader.cpp"
    "SubmissionThreadExecutor.hpp"
    "SubmissionThreadExecutor.cpp"
    "SubmissionThreadExecutionContext.hpp"
    "SubmissionThreadExecutionContext.cpp"
    "OneShotCopySubmission.hpp"
    "OneShotCopySubmission.cpp"

)

#set_source_group(
#    assets
#    "AssetLoading.hpp" 
#    "AssetLoading.cpp"
#)

set_source_group(
    objects 
    #"Camera.hpp"
    #"Camera.cpp" 
    #"Mesh.hpp"
    #"Mesh.cpp"
    #"Model.hpp"
    #"Model.cpp" 
    #"Scene.hpp" 
    #"Scene.cpp" 
)

set_source_group(
    rendering 
    "RenderingSystem.hpp" 
    "RenderingSystem.cpp"
    "SceneRenderer.hpp" 
    "SceneRenderer.cpp"
    "IPass.hpp"
    #"CameraPass.hpp"
    #"CameraPass.cpp"
    "TargetPass.hpp"
    "TargetPass.cpp"
)

add_library(
    Eureka.Graphics
    STATIC
    #${assets}
    ${infrastructure}
    #${objects}
    ${rendering}
    ${ui}
    ${renderdoc}
 )

set_target_properties(Eureka.Graphics PROPERTIES FOLDER "Libs")
configure_code_analysis(Eureka.Graphics)

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
    #${TINYGLTF_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.Graphics 
    PUBLIC
    Eureka.Interfaces 
    Eureka.Vulkan 
	Eureka.Windowing
    Pal::Sigslot
    concurrencpp::concurrencpp
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core 
    Eureka.Shaders 
    imgui::imgui #TODO to different project
    implot::implot #TODO to different project
    #unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    #unofficial::spirv-reflect::spirv-reflect
    eureka_strict_compiler_flags

)

