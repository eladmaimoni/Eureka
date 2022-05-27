cmake_minimum_required (VERSION 3.21)

include(utility.cmake)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD 14) # https://stackoverflow.com/questions/61540127/set-cxx-standard-to-c17-when-combining-c-and-cuda-in-cmakelists
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)

# set an additional search path for installed packages
if (DEFINED ENV{EXTERNALS_PATH})
    string(APPEND exported_packages_search_path "$ENV{EXTERNALS_PATH}exported_packages/installed/${VCPKG_TARGET_TRIPLET}")
    set(CMAKE_PREFIX_PATH ${exported_packages_search_path})
    message(STATUS "packages search path: ${CMAKE_PREFIX_PATH}")
else() 
    message(STATUS "EXTENALS_PATH environment variable is not found, defaulting to")
endif()


if (MSVC)
    # https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
    string(REGEX REPLACE "/W[3|4]" "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") 
endif()

project ("Eureka")

# global packages that are used system wide
find_package(Eigen3 CONFIG REQUIRED)
find_package(glfw3 "3.3.7" CONFIG REQUIRED)
#find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")
find_package(Vulkan REQUIRED FATAL_ERROR) 

# include sub-projects.
add_subdirectory (Eureka.Precompiled)
add_subdirectory (Eureka.Core)
add_subdirectory (Eureka.Graphics)
add_subdirectory (Eureka.UnitTests)


option(WITH_CUDA "include Cuda module" OFF)

if (WITH_CUDA)
  configure_cuda()
  #add_subdirectory (RGo.Cuda.Core)
  #add_subdirectory (RGo.Stereo.Cuda)
endif()


