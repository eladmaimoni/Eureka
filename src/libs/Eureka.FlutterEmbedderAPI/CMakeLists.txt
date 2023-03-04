
set(FLUTTER_EMBEDDER_API_PUBLIC_HEADERS "FlutterEmbedderAPI.h")

set_source_group(api 
    ${FLUTTER_EMBEDDER_API_PUBLIC_HEADERS}
    "FlutterEmbedderAPI.cpp"
) 

add_library(FlutterEmbedderAPI SHARED ${api})

target_compile_definitions(FlutterEmbedderAPI PRIVATE EUREKA_FLUTTER_EMBEDDER_EXPORT)

set_target_properties(FlutterEmbedderAPI PROPERTIES FOLDER "Libs")


set_target_properties(FlutterEmbedderAPI
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/flutter_api"
)



target_precompile_headers(FlutterEmbedderAPI REUSE_FROM Eureka.Precompiled)


link_library_and_copy_dynamic_library_to_output_path(
    FlutterEmbedderAPI
    flutter_engine_jit
)

target_link_libraries(
    FlutterEmbedderAPI
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
    Eureka.Flutter
    flutter_engine
    eureka_strict_compiler_flags
    #concurrencpp::concurrencpp
)

#
# export / install https://cmake.org/cmake/help/latest/guide/tutorial/Adding%20Export%20Configuration.html
#

# add the public include directories for the include
target_include_directories(
    FlutterEmbedderAPI
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> # when using within the build, the include directories is the current source dir
    $<INSTALL_INTERFACE:include> # when used from installation folder, the include directories is the exported path
)

# when installing, copy the public header files to the include directory
install(FILES ${FLUTTER_EMBEDDER_API_PUBLIC_HEADERS} DESTINATION include)

# when installing, 
# - copy the target artifacts (in this case .lib & .dll) to the lib directory withing the install path
# - create a cmake files that defines cmake targets with all relevant stuff that clients could link to

install(
    TARGETS FlutterEmbedderAPI
    EXPORT FlutterEmbedderAPI
    #DESTINATION lib/$<CONFIG>
    ARCHIVE DESTINATION lib/$<CONFIG>
    RUNTIME DESTINATION bin/$<CONFIG>
    #RUNTIME_DEPENDENCY_SET FLUTTER_EMBEDDER_API_RUNTIME_DEPENDENCY_SET
    DESTINATION lib/$<CONFIG>
)

#add_custom_command(
#    TARGET FlutterEmbedderAPI 
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        $<TARGET_FILE:FlutterEmbedderAPI> # path to the .dll
#        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> # build directory for the current configuration (e.g. Debug, Release)
#)

#install(RUNTIME_DEPENDENCY_SET FLUTTER_EMBEDDER_API_RUNTIME_DEPENDENCY_SET)

install(FILES $<TARGET_PDB_FILE:FlutterEmbedderAPI> DESTINATION lib/$<CONFIG> OPTIONAL)

# when installing, copy generated cmake targets file to the cmake directory
install(EXPORT FlutterEmbedderAPI
  FILE FlutterEmbedderAPITargets.cmake
  DESTINATION cmake
)

include(CMakePackageConfigHelpers)

# generate the config file that includes the exports
# this is for find_package to work
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/FlutterEmbedderAPIConfig.cmake"
  INSTALL_DESTINATION "cmake"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/FlutterEmbedderAPIConfig.cmake
  DESTINATION cmake
  )


# this causes the flutter_engine.dll to be copied into the bin directory
install(
  IMPORTED_RUNTIME_ARTIFACTS
  flutter_engine_jit
  DESTINATION bin
  )
#install(
#  IMPORTED_RUNTIME_ARTIFACTS
#  ${flutter_engine_jit_dll}
#  DESTINATION bin
#  )
#install(
#  FILES
#  ${flutter_engine_jit_dll}
#  DESTINATION bin
#  )