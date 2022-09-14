
add_library(Eureka.Core)

set_source_group(utils "macros.hpp" "basic_utils.hpp")
set_source_group(compiler "compiler.hpp")
set_source_group(concepts "basic_concepts.hpp")
set_source_group(debugging "debugger_trace.hpp" "debugger_trace_impl.hpp" "debugger_trace_impl.cpp" "trigger_debugger_breakpoint.hpp")
set_source_group(error "basic_errors.hpp")
set_source_group(formatting "formatter_specializations.hpp")
set_source_group(logging "logging.hpp" "logging_impl.hpp" "logging_impl.cpp")
set_source_group(containers "fixed_capacity_vector.hpp")
set_source_group(threading "thread_name.hpp" "thread_name.cpp")

if (NOT TARGET_NAME_IF_EXISTS:CUDA::nvToolsExt)
    set(perfetto_files "perfetto_tracing_session.hpp;perfetto_tracing_session.cpp;perfetto/perfetto.h;perfetto/perfetto.cc")

    if (MSVC)
	    target_compile_options(Eureka.Core PRIVATE /permissive- /bigobj) # https://github.com/google/perfetto/issues/214
	    target_compile_definitions(
          Eureka.Core 
          PUBLIC 
          PERFETTO_TRACING
	      WIN32_LEAN_AND_MEAN
	      NOMINMAX # WINDOWS: to avoid defining min/max macros
	     # _WINSOCKAPI_ # WINDOWS: to avoid re-definition in WinSock2.h
	    )
    endif()
else()
    message("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ NO PERFETTO")
endif()

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
    ${threading}
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