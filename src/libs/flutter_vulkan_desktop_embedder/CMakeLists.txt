
set(FLUTTER_EMBEDDER_API_PUBLIC_HEADERS "fvkde/fvkde.h")

set_source_group(
    cmake
    CMakeLists.txt
)

set_source_group(api 
    ${FLUTTER_EMBEDDER_API_PUBLIC_HEADERS}
    "flutter_vulkan_desktop_embedder.cpp"
) 

add_library(fvkde_jit SHARED ${api} ${cmake})
add_library(fvkde_aot SHARED ${api} ${cmake})

target_compile_definitions(fvkde_jit PRIVATE EUREKA_FLUTTER_EMBEDDER_EXPORT)
target_compile_definitions(fvkde_aot PRIVATE EUREKA_FLUTTER_EMBEDDER_EXPORT)

set_target_properties(fvkde_jit PROPERTIES FOLDER "Libs")
set_target_properties(fvkde_aot PROPERTIES FOLDER "Libs")


set_target_properties(fvkde_jit
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/flutter_api_jit"
)
set_target_properties(fvkde_aot
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/flutter_api_aot"
)


target_precompile_headers(fvkde_jit REUSE_FROM Eureka.Precompiled)
target_precompile_headers(fvkde_aot REUSE_FROM Eureka.Precompiled)


setup_copy_dynamic_library_to_output_directory(
    fvkde_jit
    flutter_engine_jit
)

setup_copy_dynamic_library_to_output_directory(
    fvkde_aot
    flutter_engine_aot
)

set(FVKDE_LINK_LIBS_COMMON 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
    Eureka.Flutter
    flutter_engine
    eureka_strict_compiler_flags
)

target_link_libraries(
    fvkde_jit
    PRIVATE 
    ${FVKDE_LINK_LIBS_COMMON}
    flutter_engine_jit
)

target_link_libraries(
    fvkde_aot
    PRIVATE 
    ${FVKDE_LINK_LIBS_COMMON}
    flutter_engine_aot
)

setup_target_for_install(fvkde_jit fvkde)
setup_target_for_install(fvkde_aot fvkde)

#set(EUREKA_INSTALL_CONFIG $<IF:$<CONFIG:Debug>,Debug,Release>)
#install(FILES $<TARGET_RUNTIME_DLLS:fvkde_jit> DESTINATION bin/${EUREKA_INSTALL_CONFIG} OPTIONAL)
#install(FILES $<TARGET_RUNTIME_DLLS:fvkde_aot> DESTINATION bin/${EUREKA_INSTALL_CONFIG} OPTIONAL)

print_generator_expression_at_build_time(TARGET_RUNTIME_DLLSCHECK $<TARGET_RUNTIME_DLLS:fvkde_jit>)

#install(
#    TARGETS fvkde_jit 
#    RUNTIME DESTINATION bin
#    RUNTIME_DEPENDENCY_SET dependency_set flutter_engine_jit
#)
#install(RUNTIME_DEPENDENCY_SET dependency_set)


########################################################################
# set available configurations
#set_property(
#    TARGET flutter_engine_jit 
#    APPEND 
#    PROPERTY IMPORTED_CONFIGURATIONS 
#    DEBUG RELEASE
#  )


# set imported location for available configurations
#set_target_properties(
#    flutter_engine_jit
#    PROPERTIES 
#	IMPORTED_LOCATION
#    $<TARGET_FILE:flutter_engine_jit>
#)

#set_target_properties(
#    flutter_engine_jit 
#    PROPERTIES 
#    IMPORTED_IMPLIB_DEBUG 
#    "${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Debug/flutter_engine.dll.lib"
#	IMPORTED_IMPLIB_RELEASE
#	"${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Release/flutter_engine.dll.lib"
#)
#
## when used in these configurations, map them to "Release" target
#set_target_properties( 
#  flutter_engine_jit PROPERTIES
#  MAP_IMPORTED_CONFIG_MINSIZEREL Release
#  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
#  )