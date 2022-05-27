cmake_minimum_required (VERSION 3.21)

# Add source to this project's executable.
add_library(
    Eureka.Core     
    "format_cpp20.hpp"
    "debugger_trace.hpp"
    "debugger_trace_impl.hpp"
    "debugger_trace_impl.cpp"
)

set_compiler_flags(Eureka.Core) 

target_include_directories(
    Eureka.Core
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    Eureka.Core 
    PUBLIC 
)