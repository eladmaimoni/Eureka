
set_source_group(api 
    "FlutterEmbedderAPI.h"
    "FlutterEmbedderAPI.cpp"
) 


add_library(FlutterEmbedderAPI SHARED ${api})

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