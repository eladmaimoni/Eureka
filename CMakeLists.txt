cmake_minimum_required (VERSION 3.21)



message("======================================================================================")
message("======================================================================================")
message("========================       PROJECT EUREKA      ===================================")
message("======================================================================================")
message("======================================================================================")

include(utility.cmake)
project ("Eureka")

############################################################################
#                            CCACHE
############################################################################




############################################################################
#                            PACKAGES
############################################################################
message("======================================================================================")
message("======================================================================================")
message("========================           PACKAGES        ===================================")
message("======================================================================================")
message("======================================================================================")
#find_program(CCACHE_FOUND ccache) 
#if(CCACHE_FOUND)
#    message("CCACHE is found")
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) # Less useful to do it for linking, see edit2
#else(CCACHE_FOUND)
#    message("CCACHE is NOT found")
#endif(CCACHE_FOUND)

add_compile_definitions(Boost_NO_WARN_NEW_VERSIONS=1)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)
message("start find package")
find_package(Boost REQUIRED COMPONENTS container)
find_package(Catch2 CONFIG REQUIRED)
find_package(concurrencpp CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(function2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(PalSigslot CONFIG REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)  
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
message("start TINYGLTF_INCLUDE_DIRS")
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
message("start CUDA")
#find_package(CUDAToolkit)
if (CUDAToolkit_FOUND)
    message("found cuda toolkit version ${CUDAToolkit_VERSION_MAJOR}.${CUDAToolkit_VERSION_MINOR}.${CUDAToolkit_VERSION_PATCH}")



endif()

#find_package(Boost REQUIRED COMPONENTS program_options)
#find_package(glslang CONFIG REQUIRED)
#find_package(SPIRV-Tools CONFIG REQUIRED)
#find_package(SPIRV-Tools-opt CONFIG REQUIRED)
#find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
#find_program(dxc_executable NAMES dxc HINTS Vulkan::dxc) points to the dxc shipped with vs

message("======================================================================================")
message("======================================================================================")
message("========================         EUREKA TARGETS    ===================================")
message("======================================================================================")
message("======================================================================================")

############################################################################
#                        GLOBAL VARIABLES
############################################################################

set(dxc_executable "$ENV{VULKAN_SDK}/Bin/dxc.exe")
configure_default_output_directories()

message("Eureka: dxc_executable = ${dxc_executable}")
message("Eureka: glslc_executable = ${glslc_executable}")


if (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Do Visual Studio specific stuff
	message("ZZZZ addinig include path ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    # it seems that vcpkg based cmake solutions don't add the vcpkg generated include directories to vcproj files' 
    include_directories("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
endif()

# HACK
if (MSVC)
    # https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
    string(REGEX REPLACE "/W[3|4]" "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") 
endif()

############################################################################
#                             SUBDIRECTORIES
############################################################################
add_subdirectory (Eureka.Precompiled)
add_subdirectory (Eureka.Core)
add_subdirectory (Eureka.Types)
add_subdirectory (Eureka.Tools)
add_subdirectory (Eureka.AssetLoading)
add_subdirectory (Eureka.Graphics)
add_subdirectory (Eureka.Shaders)
add_subdirectory (Eureka.App)
add_subdirectory (Eureka.UnitTests)
add_subdirectory (Eureka.RPC)
add_subdirectory (Eureka.RemoteProto)
add_subdirectory (Eureka.RemoteServer)
add_subdirectory (Eureka.RemoteClient)


message("======================================================================================")
message("======================================================================================")
message("========================            END            ===================================")
message("======================================================================================")
message("======================================================================================")









