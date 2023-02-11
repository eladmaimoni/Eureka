
# https://stackoverflow.com/a/72937802/10576494
# Unlike to normal targets, which are global, an IMPORTED target by default is local to the directory where it is created.

add_library(flutter_engine SHARED IMPORTED GLOBAL)

set(FLUTTER_CONFIG_DIR $<IF:$<CONFIG:Debug>,Debug,Release>)
set(FLUTTER_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin_for_dart_debug/${FLUTTER_CONFIG_DIR})

set_target_properties(
    flutter_engine 
    PROPERTIES 
    IMPORTED_LOCATION 
    "${FLUTTER_BIN_DIR}/flutter_engine.dll"
)

set_target_properties(
    flutter_engine 
    PROPERTIES 
    IMPORTED_IMPLIB_DEBUG 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_for_dart_debug/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELEASE
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_for_dart_debug/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELWITHDEBINFO
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_for_dart_debug/Debug/flutter_engine.dll.lib"
)

target_include_directories(
    flutter_engine
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

get_target_property(
    flutter_engine_imported_location
    flutter_engine 
    IMPORTED_LOCATION
)

get_target_property(
    flutter_engine_IMPORTED_IMPLIB
    flutter_engine 
    IMPORTED_IMPLIB
)
message("flutter_engine IMPORTED_LOCATION = ${flutter_engine_imported_location}")
message("flutter_engine IMPORTED_IMPLIB = ${flutter_engine_IMPORTED_IMPLIB}")

if(TARGET flutter_engine)
  message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' found")
else()
  message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' not found")
endif()