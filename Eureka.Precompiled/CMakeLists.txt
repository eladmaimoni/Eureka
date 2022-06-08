add_library(Eureka.Precompiled STATIC "precompiled.cpp" "global_definitions.hpp")

set_compiler_flags(Eureka.Precompiled)   

target_link_libraries(
  Eureka.Precompiled
  PUBLIC
  Vulkan::Vulkan
  Eigen3::Eigen
  )

target_include_directories(Eureka.Precompiled PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}")

target_compile_definitions(
    Eureka.Precompiled
    PUBLIC
    VULKAN_HPP_NO_CONSTRUCTORS
)

target_precompile_headers(
    Eureka.Precompiled 
    PRIVATE
    <algorithm>
    <array>
	<atomic>
    <bit>
    <bitset>
	<cassert>
    <chrono>
    <cmath>
    <concepts>
	<cstdint>
    <exception>
    #<iostream>
    <filesystem>
    <fstream>
	<future>
	<map>
    <memory>
	<mutex>
    <optional>
    <ranges>
    #<regex>
    <span>
    <sstream>
    <stdexcept>
    <string>
    <string_view>
    <variant>
    <vector> 
    <vulkan/vulkan.hpp>
    #<vulkan/vulkan_raii.hpp> # this messes up the pdb files !!!
    <Eigen/Core>
    <global_definitions.hpp>
 )


# set_target_properties(Eureka.Precompiled PROPERTIES OPTIMIZE_DEPENDENCIES ON LINKER_LANGUAGE CXX)


#target_link_libraries(
#  Eureka.Precompiled
#  LINK_PUBLIC
#
#  )
  #  Vulkan::Vulkan
  #Eigen3::Eigen

#     <vulkan/vulkan_raii.hpp>
#    <Eigen/Core>
#    <global_definitions.hpp>
#set_target_properties(Eureka.Precompiled PROPERTIES PRECOMPILED_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

