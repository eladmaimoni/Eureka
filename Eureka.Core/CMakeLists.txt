

set_source_group(utils "macros.hpp" "basic_utils.hpp")
set_source_group(compiler "compiler.hpp")
set_source_group(concepts "basic_concepts.hpp")
set_source_group(debugging "debugger_trace.hpp" "debugger_trace_impl.hpp" "debugger_trace_impl.cpp" "trigger_debugger_breakpoint.hpp")
set_source_group(error "basic_errors.hpp")
set_source_group(formatting "formatter_specializations.hpp")
set_source_group(logging "logging.hpp" "logging_impl.hpp" "logging_impl.cpp")
set_source_group(profiling "profiling.cpp" "profiling.hpp" "profiling_macros.hpp")
set_source_group(containers "fixed_capacity_vector.hpp")




add_library(
    Eureka.Core     
    ${compiler}
    ${concepts}
    ${debugging}
    ${error}
    ${formatting}
    ${profiling}
    ${utils}
    ${containers}
)

set_compiler_flags(Eureka.Core) 

target_compile_definitions( # temporary hack, profiling is always enabled
    Eureka.Core
    PUBLIC 
    PROFILING_ENABLED
    PRIVATE
    $<IF:$<TARGET_EXISTS:CUDA::nvToolsExt>,EUREKA_HAS_NVTOOLSEXT,EUREKA_NO_NVTOOLSEXT>
)

target_include_directories(
    Eureka.Core
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    Eureka.Core 
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:CUDA::nvToolsExt>
    #"$<$<CUDAToolkit_FOUND>:CUDA::nvToolsExt>"    
)