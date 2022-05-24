add_library(Eureka.Precompiled STATIC "precompiled.cpp")
set_target_properties(Eureka.Precompiled PROPERTIES OPTIMIZE_DEPENDENCIES ON LINKER_LANGUAGE CXX)
target_include_directories(Eureka.Precompiled PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC "${PROJECT_BINARY_DIR}")
set_compiler_flags(Eureka.Precompiled)    

target_compile_definitions(
    Eureka.Precompiled
    PUBLIC
    VULKAN_HPP_NO_CONSTRUCTORS
)

target_link_libraries(
  Eureka.Precompiled
  LINK_PUBLIC
  Vulkan::Vulkan
  Eigen3::Eigen
  )

#[[
Notes regarding precompiled headers

What can be added here?
- header only libraries (3rd party or not, just make sure they have no cpp)
- libraries that are included often, and by most targets. don't put here heavy headers
  that are included just by your target.
- be judicious about what you include here:
  good: <opencv/core.hpp> // include just the bare minimum that everybody use
  bad: <opencv/opencv.hpp> // include everything, let everybody pay the price

How to use in my target?
- make sure you compilation flags are identical to the ones on this target
- use target_precompile_headers with the REUSE_FROM flag
There are 2 helper functions to do that for you:
set_strict_msvc_flags(my_target)
target_shared_precompiled_headers(my_target Eureka.Precompiled)

]]
target_precompile_headers(Eureka.Precompiled 
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
    <iostream>
    <filesystem>
    <fstream>
	<future>
	<map>
    <memory>
	<mutex>
    <optional>
    <ranges>
    <regex>
    <sstream>
    <stdexcept>
    <string>
    <string_view>
    <variant>
    <vector> 
    <vulkan/vulkan_raii.hpp>
    <Eigen/Core>
 )

set_target_properties(Eureka.Precompiled PROPERTIES PRECOMPILED_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

