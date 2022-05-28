

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
function(configure_shader_compilation shader_file)
    get_source_file_property(shader_type ${shader_file} ShaderType)    
    get_filename_component(shader_name ${shader_file} NAME_WE)
    message("glslc_executable = ${glslc_executable}")
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

function(configure_shaders_compilation)
    foreach(shader IN LISTS ARGN)
        configure_shader_compilation(${shader})
    endforeach()
endfunction()

macro(shaders_group group_name shader_type)
    set(list_name ${group_name}_generated_headers_list)
    list(APPEND ${list_name} "")
    foreach(shader_file IN ITEMS ${ARGN})
        get_filename_component(shader_name ${shader_file} NAME_WE)
        list(APPEND ${list_name} "${EUREKA_COMPILED_SHADERS_DIRECTORY}/${shader_name}.spvhpp")
        set_source_files_properties(${shader_file} PROPERTIES ShaderType ${shader_type})
    endforeach()
    set(${group_name} "${ARGN}")  
    source_group(${group_name} FILES ${ARGN}) 
    message("vertex_shaders_generated_headers_list = ${list_name}")
endmacro(shaders_group)

################################################################################
#                                  Shaders
################################################################################
shaders_group(vertex_shaders vertex "Shader1.hlsl" "ShadedMeshVS.hlsl")


################################################################################
#                              Shaders Targets
################################################################################
add_custom_target(
    Eureka.ShadersTarget 
    DEPENDS
    ${vertex_shaders_generated_headers_list}
)

configure_shaders_compilation(${vertex_shaders})


add_library(
    Eureka.Shaders 
    STATIC 
    ${vertex_shaders}
    "ShadersCache.cpp"
    "ShadersCache.hpp"
    "ShadersDeclare.hpp"
    "ShadersImpl.cpp"
)

set_compiler_flags(Eureka.Shaders)

target_include_directories(
    Eureka.Shaders
    PRIVATE
    ${EUREKA_COMPILED_SHADERS_DIRECTORY}
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(Eureka.Shaders Eureka.ShadersTarget)


target_shared_precompiled_headers(
    Eureka.Shaders 
    Eureka.Precompiled
)

target_link_libraries(
    Eureka.Shaders
    PRIVATE
    Eureka.Core
    Eureka.Precompiled
)