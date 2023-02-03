
set_source_group(
    windowing 
    "GLFWRuntime.hpp" 
    "GLFWRuntime.cpp" 
    "Window.hpp" 
    "Window.cpp" 
    "WindowTypes.hpp" 
)

add_library(
    Eureka.Windowing
    ${windowing}
 )


target_include_directories(
    Eureka.Windowing
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE 
    ${TINYGLTF_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.Windowing 
    PUBLIC
    PRIVATE 
    Eureka.Core 
    glfw
    volk::volk 
    volk::volk_headers	 
	eureka_strict_compiler_flags

)

