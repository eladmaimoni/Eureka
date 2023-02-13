
# https://stackoverflow.com/a/72937802/10576494
# Unlike to normal targets, which are global, an IMPORTED target by default is local to the directory where it is created.

add_library(flutter_engine INTERFACE)
add_library(flutter_engine_jit SHARED IMPORTED GLOBAL)
add_library(flutter_engine_aot SHARED IMPORTED GLOBAL)

target_include_directories(
    flutter_engine
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#set_property(
#  TARGET flutter_engine_jit APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG
#  )


set_target_properties(
    flutter_engine_jit
    PROPERTIES 
	IMPORTED_LOCATION
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Release/flutter_engine.dll"
	IMPORTED_LOCATION_DEBUG 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Debug/flutter_engine.dll"
)

set_target_properties(
    flutter_engine_jit 
    PROPERTIES 
    IMPORTED_IMPLIB_DEBUG 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELEASE
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Release/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELWITHDEBINFO
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_jit/Release/flutter_engine.dll.lib"
)

set_target_properties(
    flutter_engine_aot
    PROPERTIES 
	IMPORTED_LOCATION 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_aot/Release/flutter_engine.dll"
    IMPORTED_LOCATION_DEBUG 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_aot/Debug/flutter_engine.dll"

)

set_target_properties(
    flutter_engine_aot 
    PROPERTIES 
    IMPORTED_IMPLIB_DEBUG 
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_aot/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELEASE
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_aot/Release/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELWITHDEBINFO
	"${CMAKE_CURRENT_SOURCE_DIR}/bin_aot/Release/flutter_engine.dll.lib"
)

#get_target_property(
#    flutter_engine_imported_location
#    flutter_engine 
#    IMPORTED_LOCATION
#)
#
#get_target_property(
#    flutter_engine_IMPORTED_IMPLIB
#    flutter_engine 
#    IMPORTED_IMPLIB
#)
#message("flutter_engine IMPORTED_LOCATION = ${flutter_engine_imported_location}")
#message("flutter_engine IMPORTED_IMPLIB = ${flutter_engine_IMPORTED_IMPLIB}")

#if(TARGET flutter_engine)
#  message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' found")
#else()
#  message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' not found")
#endif()