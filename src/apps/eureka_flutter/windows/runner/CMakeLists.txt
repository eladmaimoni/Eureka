cmake_minimum_required(VERSION 3.25)

set_source_group(
    run
    main.cpp
)

set_source_group(
    resources
    "Runner.rc"
    "runner.exe.manifest"
)

add_executable(${BINARY_NAME} WIN32
    ${run}
    ${resources}
    #"${FLUTTER_MANAGED_DIR}/generated_plugin_registrant.cc" # TODO
)

if (EUREKA_FLUTTER_IS_A_SUBDIRECTORY)
    set_target_properties(${BINARY_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eureka_flutter
    )
endif()

target_link_libraries(
    ${BINARY_NAME}
    PRIVATE
    eureka_strict_compiler_flags
    fvkde_jit
    flutter_engine_jit
)

add_dependencies(${BINARY_NAME} flutter_assemble_jit)

#setup_copy_dynamic_library_to_output_directory(${BINARY_NAME} flutter_engine_jit)
setup_copy_runtime_dlls_to_output_directory(${BINARY_NAME})
#print_generator_expression_at_build_time(DLL_DEBUG $<TARGET_RUNTIME_DLLS:${BINARY_NAME}>)

#message("ZZZZZZZZZZZZ fvkde_jit_RELEASE = ${fvkde_jit_RELEASE}")
#message("ZZZZZZZZZZZZ fvkde_jit_DEBUG = ${fvkde_jit_DEBUG}")
#print_generator_expression_at_build_time(fvkde_jit_dll_location $<TARGET_FILE:fvkde_jit>)

# Add preprocessor definitions for the build version.
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION=\"${FLUTTER_VERSION}\"")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_MAJOR=${FLUTTER_VERSION_MAJOR}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_MINOR=${FLUTTER_VERSION_MINOR}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_PATCH=${FLUTTER_VERSION_PATCH}")
target_compile_definitions(${BINARY_NAME} PRIVATE "FLUTTER_VERSION_BUILD=${FLUTTER_VERSION_BUILD}")


# Run the Flutter tool portions of the build. This must not be removed.

