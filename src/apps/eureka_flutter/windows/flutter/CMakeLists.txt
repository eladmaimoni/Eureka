# This file controls Flutter-level build steps. It should not be edited.
cmake_minimum_required(VERSION 3.25)
set(EPHEMERAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ephemeral")

# Configuration provided via flutter tool.
include(generated_plugins.cmake)
include(ephemeral/generated_config.cmake)

# published to parent scope for install step.
# this is where vscode builds stuff, if we change it, we are doomed as this is a global flutter configuration
set(PROJECT_BUILD_DIR "${PROJECT_DIR}/build/" PARENT_SCOPE)  
set(FLUTTER_ICU_DATA_FILE "${EPHEMERAL_DIR}/icudtl.dat" PARENT_SCOPE)
set(AOT_LIBRARY "${PROJECT_DIR}/build/windows/app.so" PARENT_SCOPE)



set(FLUTTER_FILE_THAT_SURELY_CHANGES ${EUREKA_FLUTTER_ROOT}/build/flutter_assets/stamp.txt)
file(GLOB_RECURSE DART_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/*.dart")
message("DART_FILES = ${DART_FILES}")
message("FLUTTER_FILE_THAT_SURELY_CHANGES = ${FLUTTER_FILE_THAT_SURELY_CHANGES}")
message("FLUTTER_TOOL_ENVIRONMENT = ${FLUTTER_TOOL_ENVIRONMENT}")

add_custom_command(
  OUTPUT ${FLUTTER_FILE_THAT_SURELY_CHANGES}
  DEPENDS ${DART_FILES}
  COMMAND ${CMAKE_COMMAND} -E env
    ${FLUTTER_TOOL_ENVIRONMENT}
    "${FLUTTER_ROOT}/packages/flutter_tools/bin/tool_backend.bat" # this invokes 'flutter assemble commend'
      windows-x64 # build for windows
      #$<CONFIG>
      Debug # build we always build Debug instead of $<CONFIG> configuration since this is a jit target for VS Code debugging
  COMMAND ${CMAKE_COMMAND} -E touch ${FLUTTER_FILE_THAT_SURELY_CHANGES}
  VERBATIM
)

add_library(
    flutter_assemble_jit 
    INTERFACE 
    ${FLUTTER_FILE_THAT_SURELY_CHANGES}
)

