

# relvant material on adding shaders via cmake:
# https://stackoverflow.com/questions/57478571/why-wont-cmake-build-my-vulkan-spirv-shaders
# https://stackoverflow.com/questions/60420700/cmake-invocation-of-glslc-with-respect-to-includes-dependencies
# https://stackoverflow.com/questions/61387495/how-to-compile-hlsl-shaders-with-vulkan
# https://stackoverflow.com/questions/18427877/add-custom-build-step-in-cmake
# https://www.reddit.com/r/vulkan/comments/kbaxlz/what_is_your_workflow_when_compiling_shader_files/
# https://github.com/katsuNakajima/GLSL-objcopy/blob/master/CMakeLists.txt 
# glslc -fshader-stage=vertex Shader1.hlsl -o Shader.spv 

################################################################################
#                        CMAKE Shaders Helpers
################################################################################
function(configure_glsl_shader_compilation shader_file)
    get_source_file_property(shader_type ${shader_file} ShaderType)    
    get_filename_component(shader_name ${shader_file} NAME_WE)
    
    message("raw shader file = ${shader_file}")
    message("shader name = ${shader_name}")
    message("shader type = ${shader_type}")
    message("out = ${CMAKE_CURRENT_SOURCE_DIR}/${source}")

    add_custom_command(
        #PRE_BUILD
        #TARGET Eureka.Shaders
        #TARGET shaders_custom_target
        OUTPUT 
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spvhpp # absolute path to the generated header
        #${shader_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader_file} # the input to the command
        #MAIN_DEPENDENCY ${shader_file} # tried with and without

        COMMAND
        ${glslc_executable}
        -fshader-stage=${shader_type} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}
        -o
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spv
        COMMAND
        bin2h
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spv
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
     )
endfunction()

function(configure_hlsl_shader_compilation shader_file)
    get_source_file_property(shader_type ${shader_file} ShaderType)    
    get_filename_component(shader_name ${shader_file} NAME_WE)
    message("raw shader file = ${shader_file}")
    message("shader name = ${shader_name}")
    message("shader type = ${shader_type}")
    message("out = ${CMAKE_CURRENT_SOURCE_DIR}/${source}")

    add_custom_command(
        #PRE_BUILD
        #TARGET Eureka.Shaders
        #TARGET shaders_custom_target
        OUTPUT 
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spvhpp # absolute path to the generated header
        #${shader_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader_file} # the input to the command
        #MAIN_DEPENDENCY ${shader_file} # tried with and without

        COMMAND
        ${dxc_executable} -spirv -HV 2021 -E main -fspv-target-env=vulkan1.2 
        -T ${shader_type}        
        ${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}
        -Fo
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spv
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND
        bin2h
        ${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spv
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
     )
endfunction()


function(configure_glsl_shaders_compilation)
    message("glslc_executable = ${glslc_executable}")
    foreach(shader IN LISTS ARGN)
        configure_glsl_shader_compilation(${shader})
    endforeach()
endfunction()

function(configure_hlsl_shaders_compilation)
    message("dxc_executable = ${dxc_executable}")
    # prevent visual studio hlsl compilation
    # https://stackoverflow.com/questions/43739539/cmake-avoid-compilation-of-hlsl-shader-files-on-visual-studio
    set_source_files_properties(${ARGN} PROPERTIES VS_TOOL_OVERRIDE "None")
    foreach(shader IN LISTS ARGN)
        #set_source_files_properties(${shader} PROPERTIES VS_TOOL_OVERRIDE "None")
        configure_hlsl_shader_compilation(${shader})
    endforeach()
endfunction()

macro(shaders_group group_name shader_type)
    #message("MACRO ARGS : group_name:${group_name} shader_type:${shader_type} ARGN:${ARGN}  ARGV:${ARGV}")
    set(list_name ${group_name}_generated_headers_list)
    list(APPEND ${list_name} "")
    foreach(shader_file IN ITEMS ${ARGN})
        get_filename_component(shader_name ${shader_file} NAME_WE)
        list(APPEND ${list_name} "${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spvhpp")
        set_source_files_properties(${shader_file} PROPERTIES ShaderType ${shader_type})
    endforeach()
    set(${group_name} "${ARGN}")  
    source_group(${group_name} FILES ${ARGN}) 
    message("generated shaders generated headers list named : ${list_name}")
endmacro(shaders_group)

################################################################################
#                                  Shaders
################################################################################
shaders_group(glsl_vertex_shaders vertex "ImGuiGLSLVS.vert" "TriangleVS.vert")
shaders_group(glsl_fragment_shaders fragment "ImGuiGLSLFS.frag" "TriangleFS.frag") 
shaders_group(hlsl_fragment_shaders ps_6_0 "Textured2DRegionFS.hlsl" "ImGuiFS.hlsl" "ColoredVertexFS.hlsl" "PhongShadedMeshWithNormalMapFS.hlsl")
shaders_group(hlsl_vertex_shaders vs_6_0 "Textured2DRegionVS.hlsl" "ImGuiVS.hlsl" "ColoredVertexVS.hlsl" "ShadedMeshWithNormalMapVS.hlsl")


################################################################################
#                              Shaders Targets
################################################################################
#add_custom_target(
#    Eureka.ShadersTarget 
#    DEPENDS
#    ${hlsl_vertex_shaders_generated_headers_list}
#    ${hlsl_fragment_shaders_generated_headers_list}
#    ${glsl_vertex_shaders_generated_headers_list}
#    ${glsl_fragment_shaders_generated_headers_list}
#)

configure_glsl_shaders_compilation(${glsl_vertex_shaders})
configure_glsl_shaders_compilation(${glsl_fragment_shaders})
configure_hlsl_shaders_compilation(${hlsl_vertex_shaders})
configure_hlsl_shaders_compilation(${hlsl_fragment_shaders})

set_source_group(
    common
    "set0_id_000.hlsli"
)

set_source_group(
    shader_cache 
    "ShadersDeclare.hpp"
    "ShadersImpl.cpp"
)

set_source_group(
    generated_files
    ${hlsl_vertex_shaders_generated_headers_list}
    ${hlsl_fragment_shaders_generated_headers_list}
    ${glsl_vertex_shaders_generated_headers_list}
    ${glsl_fragment_shaders_generated_headers_list}
)

add_library(
    Eureka.Shaders 
    STATIC 
    ${common}
    ${shader_cache}
    # shader files
    ${hlsl_vertex_shaders}
    ${hlsl_fragment_shaders}
    ${glsl_vertex_shaders}
    ${glsl_fragment_shaders}
    # generated headers
    ${generated_files}

)

target_include_directories(
    Eureka.Shaders
    PRIVATE
    ${EUREKA_COMPILED_SHADERS_DIRECTORY}
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_dependencies(Eureka.Shaders Eureka.ShadersTarget)

target_link_libraries(
    Eureka.Shaders
    PRIVATE
    Eureka.Core
    Eureka.Vulkan
    eureka_strict_compiler_flags
)