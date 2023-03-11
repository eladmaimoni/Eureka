
# https://stackoverflow.com/a/72937802/10576494
# Unlike to normal targets, which are global, an IMPORTED target by default is local to the directory where it is created.
# https://cmake.org/cmake/help/latest/prop_tgt/MAP_IMPORTED_CONFIG_CONFIG.html#prop_tgt:MAP_IMPORTED_CONFIG_%3CCONFIG%3E
#######################################################################################################################
# flutter_engine (headers)
#######################################################################################################################

add_library(flutter_engine INTERFACE)

target_include_directories(
    flutter_engine
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#######################################################################################################################
# flutter_engine_jit 
#######################################################################################################################

add_library(flutter_engine_jit SHARED IMPORTED GLOBAL)

# set available configurations
set_property(
    TARGET flutter_engine_jit 
    APPEND 
    PROPERTY IMPORTED_CONFIGURATIONS 
    DEBUG RELEASE
  )

# set imported location for available configurations
set_target_properties(
    flutter_engine_jit
    PROPERTIES 
	IMPORTED_LOCATION_DEBUG 
    "${CMAKE_CURRENT_LIST_DIR}/bin_jit/Debug/flutter_engine.dll"
    IMPORTED_LOCATION_RELEASE 
    "${CMAKE_CURRENT_LIST_DIR}/bin_jit/Release/flutter_engine.dll"
)

set_target_properties(
    flutter_engine_jit 
    PROPERTIES 
    IMPORTED_IMPLIB_DEBUG 
    "${CMAKE_CURRENT_LIST_DIR}/bin_jit/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELEASE
	"${CMAKE_CURRENT_LIST_DIR}/bin_jit/Release/flutter_engine.dll.lib"
)

# when used in these configurations, map them to "Release" target
set_target_properties( 
  flutter_engine_jit PROPERTIES
  MAP_IMPORTED_CONFIG_MINSIZEREL Release
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
  )



# see this CMAKE issue: https://gitlab.kitware.com/cmake/cmake/-/issues/22316
# and this https://cmake.org/cmake/help/v3.20/module/BundleUtilities.html
#setup_target_includes_for_install(flutter_engine_jit flutter)


#get_property(
#  flutter_engine_jit_imported_configs 
#  TARGET flutter_engine_jit 
#  PROPERTY IMPORTED_CONFIGURATIONS
#  )
#
#message("ZZZZZZZZZZZZ flutter_engine_jit IMPORTED_CONFIGURATIONS = ${flutter_engine_jit_imported_configs}\n")

# set available configurations for imported target


#get_property(
#  flutter_engine_jit_imported_configs 
#  TARGET flutter_engine_jit 
#  PROPERTY IMPORTED_CONFIGURATIONS
#  )
#
#message("ZZZZZZZZZZZZ flutter_engine_jit IMPORTED_CONFIGURATIONS = ${flutter_engine_jit_imported_configs}\n")



#######################################################################################################################
# flutter_engine_aot 
#######################################################################################################################

add_library(flutter_engine_aot SHARED IMPORTED GLOBAL)

# set available configurations
set_property(
    TARGET flutter_engine_aot 
    APPEND 
    PROPERTY IMPORTED_CONFIGURATIONS 
    DEBUG RELEASE
  )

# set imported location for available configurations
set_target_properties(
    flutter_engine_aot
    PROPERTIES 
	IMPORTED_LOCATION_DEBUG 
    "${CMAKE_CURRENT_LIST_DIR}/bin_aot/Debug/flutter_engine.dll"
    IMPORTED_LOCATION_RELEASE 
    "${CMAKE_CURRENT_LIST_DIR}/bin_aot/Release/flutter_engine.dll"
)

set_target_properties(
    flutter_engine_aot 
    PROPERTIES 
    IMPORTED_IMPLIB_DEBUG 
    "${CMAKE_CURRENT_LIST_DIR}/bin_aot/Debug/flutter_engine.dll.lib"
	IMPORTED_IMPLIB_RELEASE
	"${CMAKE_CURRENT_LIST_DIR}/bin_aot/Release/flutter_engine.dll.lib"
)

# when used in these configurations, map them to "Release" target
set_target_properties( 
  flutter_engine_aot PROPERTIES
  MAP_IMPORTED_CONFIG_MINSIZEREL Release
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
  )




#setup_target_compiled_artifacts_for_install(flutter_engine_jit)
#setup_target_includes_for_install(flutter_engine_aot flutter)
#setup_target_for_find_package(flutter_engine_aot)
#setup_target_compiled_artifacts_for_install(flutter_engine_aot)
