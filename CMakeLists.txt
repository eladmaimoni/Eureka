cmake_minimum_required (VERSION 3.21)

include(utility.cmake)

############################################################################
#                          CCOMPILER STANDARDS
############################################################################
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED True)



project ("Eureka")


############################################################################
#                            PACKAGES
############################################################################
find_package(Catch2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(glm CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(function2 CONFIG REQUIRED)
#find_package(glslang CONFIG REQUIRED)
#find_package(SPIRV-Tools CONFIG REQUIRED)
#find_package(SPIRV-Tools-opt CONFIG REQUIRED)
#find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
find_package(Vulkan REQUIRED FATAL_ERROR COMPONENTS glslc dxc)  
#find_package(unofficial-shaderc_util CONFIG REQUIRED)
#find_package(unofficial-shaderc CONFIG REQUIRED)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
#find_program(dxc_executable NAMES dxc HINTS Vulkan::dxc) points to the dxc shipped with vs
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

message("======================================================================================")
message("======================================================================================")
message("========================           START           ===================================")
message("======================================================================================")
message("======================================================================================")

set(dxc_executable "$ENV{VULKAN_SDK}/Bin/dxc.exe")
message("Eureka: dxc_executable = ${dxc_executable}")
message("Eureka: dxc_executable = $ENV{VULKAN_SDK}/Bin/dxc.exe")

message("Eureka: glslc_executable = ${glslc_executable}")


############################################################################
#                        GLOBAL VARIABLES
############################################################################

#set_target_properties( yourexe PROPERTIES RUNTIME_OUTPUT_DIRECTORY E:/parsec/bin/ )

# include sub-projects.

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if (MSVC)
    # https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
    string(REGEX REPLACE "/W[3|4]" "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") 
endif()
configure_default_output_directories()

add_subdirectory (Eureka.Precompiled)
add_subdirectory (Eureka.Tools)
add_subdirectory (Eureka.Core)
add_subdirectory (Eureka.Graphics)
add_subdirectory (Eureka.Shaders)
add_subdirectory (Eureka.App)
add_subdirectory (Eureka.UnitTests)
#add_subdirectory (ZZZ_SAMPLE)

message("======================================================================================")
message("======================================================================================")
message("========================            END            ===================================")
message("======================================================================================")
message("======================================================================================")









