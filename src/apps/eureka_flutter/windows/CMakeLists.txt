################################################################################################
################################################################################################
# root cmake for flutter project that is invoked by VS Code
# we should be careful when changing this project and its subdirectories
# VS code typically calls the INSTALL target of this project which copies compiles and copies 
# flutter stuff to the executable directory
################################################################################################
################################################################################################
cmake_minimum_required(VERSION 3.25)



message(STATUS
"\n======================================================================================"
"\n======================================================================================"
"\n=====================       PROJECT EUREKA FLUTTER       ============================="
"\n======================================================================================"
"\n======================================================================================"
)
if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR 
    "VCPKG_ROOT envrionment variable is not defined.\n"
    "please install vcpkg and set the appropriate environment variable."    
    )
endif()

set(EUREKA_PARENT_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)
set(EUREKA_FLUTTER_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(EUREKA_INSTALLATION_DIR ${EUREKA_PARENT_SOURCE_ROOT}/install/${CMAKE_HOST_SYSTEM_NAME})

if (NOT EUREKA_FLUTTER_IS_A_SUBDIRECTORY)
    # setup vcpkg
    #set(VCPKG_TARGET_TRIPLET x64-windows-static-md)
    #set(VCPKG_MANIFEST_MODE ON)
    #set(VCPKG_MANIFEST_DIR ${EUREKA_PARENT_SOURCE_ROOT})
    #set(VCPKG_INSTALLED_DIR ${EUREKA_PARENT_SOURCE_ROOT}/build/packages)
    #include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()
###################################################################################
# ROOT CMAKE FOR FLUTTER APP
###################################################################################

# Project-level configuration.
project(eureka_flutter LANGUAGES CXX)


include(${EUREKA_PARENT_SOURCE_ROOT}/src/3rdParty/flutter/CMakeLists.txt)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/code_analysis.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/compiler_flags.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/debugging_utils.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/dynamic_library_link.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/global_setup.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/installation.cmake)
include(${EUREKA_PARENT_SOURCE_ROOT}/cmake/target_helpers.cmake)

add_library(
    eureka_flutter.configuration
    INTERFACE
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/code_analysis.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/compiler_flags.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/debugging_utils.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/dynamic_library_link.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/global_setup.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/installation.cmake
    ${EUREKA_PARENT_SOURCE_ROOT}/cmake/target_helpers.cmake
    cmake/flutter_utils.cmake
    flutter/generated_plugins.cmake
    flutter/ephemeral/generated_config.cmake
)

#if (EUREKA_FLUTTER_IS_A_SUBDIRECTORY)
#    configure_default_output_directories_relative(${EUREKA_PARENT_SOURCE_ROOT}/build/eureka_flutter)
#endif()

include(cmake/flutter_utils.cmake)

#include(flutter/ephemeral/generated_config.cmake)

# this is instead of hard coding this option in VS Code "Flutter Additional Args --local-engine=host_debug"
#set(FLUTTER_ENGINE_CONFIGURATION $<IF:$<CONFIG:Debug>,host_debug,host_release>)
#set(FLUTTER_ENGINE_CONFIGURATION $<IF:$<CONFIG:Debug>,host_debug,host_release>)

#list(APPEND FLUTTER_TOOL_ENVIRONMENT
#  "LOCAL_ENGINE=${FLUTTER_ENGINE_CONFIGURATION}"
#)



message(STATUS
"\n======================================================================================"
"\n======================================================================================"
"\n========================           PACKAGES        ==================================="
"\n======================================================================================"
"\n======================================================================================"
)

list(PREPEND CMAKE_PREFIX_PATH
	${EUREKA_INSTALLATION_DIR}
)

find_package(fvkde_jit CONFIG REQUIRED) # imported targets are only Debug and RelWithDeInfo
find_package(fvkde_aot CONFIG REQUIRED)

# when used from vs code to debug flutter dart app, we normally want
# to use the release version of the flutter_engine.dll and fvkde 
# force usage of release version of the library

set_target_properties( 
  flutter_engine_jit PROPERTIES
  MAP_IMPORTED_CONFIG_DEBUG Release
  MAP_IMPORTED_CONFIG_MINSIZEREL Release
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
  )

set_target_properties( 
  fvkde_jit PROPERTIES
  MAP_IMPORTED_CONFIG_DEBUG RelWithDebInfo
  MAP_IMPORTED_CONFIG_MINSIZEREL RelWithDebInfo
  MAP_IMPORTED_CONFIG_RELEASE RelWithDebInfo
  )

get_property(
	available_configurations
    TARGET fvkde_jit
    PROPERTY IMPORTED_CONFIGURATIONS 
  )


message("fvkde_jit IMPORTED_CONFIGURATIONS = ${available_configurations}")

############################################################################
#                        GLOBAL VARIABLES
############################################################################

# The name of the executable created for the application. Change this to change
# the on-disk name of your application.
set(BINARY_NAME "eureka_flutter")

# Explicitly opt in to modern CMake behaviors to avoid warnings with recent
# versions of CMake.
cmake_policy(SET CMP0063 NEW)

set_available_project_configurations(Debug Profile Release)
set_profile_mode_default_flags() # Define settings for the Profile build mode.

# Use Unicode for all projects.
add_definitions(-DUNICODE -D_UNICODE)

message(STATUS
"\n======================================================================================"
"\n======================================================================================"
"\n======================       EUREKA FLUTTER TARGETS      ============================="
"\n======================================================================================"
"\n======================================================================================"
)

add_subdirectory(flutter) # Flutter library and tool build rules.
add_subdirectory(runner) # flutter application runner

# Generated plugin build rules, which manage building the plugins and adding
# them to the application.














# === Installation ===
# Support files are copied into place next to the executable, so that it can
# run in place. This is done instead of making a separate bundle (as on Linux)
# so that building and running from within Visual Studio will work.

# this is where the executable located
set(BUILD_BUNDLE_DIR "$<TARGET_FILE_DIR:${BINARY_NAME}>") 
# Make the "install" step default, as it's required to run.
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)

# here we force CMAKE_INSTALL_PREFIX to point to the executable location
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${BUILD_BUNDLE_DIR}" CACHE PATH "..." FORCE)
  #set(CMAKE_INSTALL_PREFIX "${BUILD_BUNDLE_DIR}" CACHE PATH "..." FORCE)
endif()

# INSTALL_BUNDLE_DATA_DIR is where icudtl.dat will be copied along with flutter_assets dir that is generated by vscode 
set(INSTALL_BUNDLE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(INSTALL_BUNDLE_LIB_DIR "${CMAKE_INSTALL_PREFIX}")

# copy stuff to the executable directory
install(TARGETS ${BINARY_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
  COMPONENT Runtime)

install(FILES "${FLUTTER_ICU_DATA_FILE}" DESTINATION "${INSTALL_BUNDLE_DATA_DIR}"
  COMPONENT Runtime)


if(PLUGIN_BUNDLED_LIBRARIES)
  install(FILES "${PLUGIN_BUNDLED_LIBRARIES}"
    DESTINATION "${INSTALL_BUNDLE_LIB_DIR}"
    COMPONENT Runtime)
endif()

# Fully re-copy the assets directory on each build to avoid having stale files
# from a previous install.
set(FLUTTER_ASSET_DIR_NAME "flutter_assets")
install(CODE "
  file(REMOVE_RECURSE \"${INSTALL_BUNDLE_DATA_DIR}/${FLUTTER_ASSET_DIR_NAME}\")
  " COMPONENT Runtime)
install(DIRECTORY "${PROJECT_BUILD_DIR}/${FLUTTER_ASSET_DIR_NAME}"
  DESTINATION "${INSTALL_BUNDLE_DATA_DIR}" COMPONENT Runtime)

## Install the AOT library on non-Debug builds only.
#install(FILES "${AOT_LIBRARY}" DESTINATION "${INSTALL_BUNDLE_DATA_DIR}"
#  CONFIGURATIONS Profile;Release
#  COMPONENT Runtime)
