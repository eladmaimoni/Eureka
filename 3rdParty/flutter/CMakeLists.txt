
# https://stackoverflow.com/a/72937802/10576494
# Unlike to normal targets, which are global, an IMPORTED target by default is local to the directory where it is created.

add_library(flutter_engine SHARED IMPORTED GLOBAL)


set_target_properties(
    flutter_engine 
    PROPERTIES 
    IMPORTED_LOCATION 
    ${CMAKE_CURRENT_SOURCE_DIR}/flutter_engine.dll
)

set_target_properties(
    flutter_engine 
    PROPERTIES 
    IMPORTED_IMPLIB  
    ${CMAKE_CURRENT_SOURCE_DIR}/flutter_engine.dll.lib
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