add_subdirectory(flutter)


if(TARGET flutter_engine)
   message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' found")
else()
   message("${CMAKE_CURRENT_SOURCE_DIR}: Target 'flutter_engine' not found")
endif()