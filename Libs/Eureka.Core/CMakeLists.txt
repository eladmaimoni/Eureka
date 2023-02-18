
add_library(Eureka.Core)

if (NOT TARGET_NAME_IF_EXISTS:CUDA::nvToolsExt)
    set(perfetto_files "perfetto_tracing_session.hpp;perfetto_tracing_session.cpp;perfetto/perfetto.h;perfetto/perfetto.cc")

    if (MSVC)
	    target_compile_options(Eureka.Core PRIVATE /permissive- /bigobj) # https://github.com/google/perfetto/issues/214
	    target_compile_definitions(
          Eureka.Core 
          PUBLIC 
          PERFETTO_TRACING
	    )
    endif()
else()
    message("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ NO PERFETTO")
endif()

set_source_group(utils "move.hpp" "macros.hpp" "basic_utils.hpp")
set_source_group(compiler "compiler.hpp")
set_source_group(concepts "basic_concepts.hpp")
set_source_group(debugging "debugger_trace.hpp" "debugger_trace_impl.hpp" "debugger_trace_impl.cpp" "trigger_debugger_breakpoint.hpp")
set_source_group(error "basic_errors.hpp" "assert.hpp")
set_source_group(formatting "formatter_specializations.hpp")
set_source_group(logging "logging.hpp" "logging_impl.hpp" "logging_impl.cpp")
set_source_group(containers "containers_aliases.hpp" "fixed_capacity_vector.hpp")
set_source_group(os "system.hpp" "system.cpp" "thread_name.hpp" "thread_name.cpp" "windows.hpp" "future.hpp")

set_source_group(profiling 
    "profiling.cpp" 
    "profiling.hpp"
    "profiling_macros.hpp"
    "profiling_categories.hpp"
    "profiling_categories.cpp"
    ${perfetto_files}
)

target_sources(
    Eureka.Core  
    PRIVATE
    ${compiler}
    ${concepts}
    ${debugging}
    ${error}
    ${formatting}
    ${profiling}
    ${utils}
    ${containers}
    ${logging}
    ${os}
)

set_target_properties(Eureka.Core PROPERTIES FOLDER "Libs")

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
    eureka_strict_compiler_flags
    #"$<$<CUDAToolkit_FOUND>:CUDA::nvToolsExt>"    
)

if(WIN32)
  target_link_libraries(Eureka.Core INTERFACE wsock32 ws2_32) # for perfetto tracing, clients must link winsock
endif()