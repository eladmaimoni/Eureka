

set_source_group(debugging "debugger_trace.hpp" "debugger_trace_impl.hpp" "debugger_trace_impl.cpp")
set_source_group(logging "logging.hpp" "logging_impl.hpp" "logging_impl.cpp")

set_source_group(formatting "formatter_specializations.hpp")
set_source_group(compiler "compiler.hpp")
set_source_group(concepts "basic_concepts.hpp")



add_library(
    Eureka.Core     
    ${debugging}
    ${formatting}
    ${compiler}
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