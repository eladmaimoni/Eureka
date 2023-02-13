#set_source_group(
#    flutter 
#    #"flutter_embedding_tests.cpp"
#    #"flutter_embedding_google_sample.cpp"
#)


set_source_group(
    run 
    "main.cpp"
)
add_executable(
    FlutterAppJIT
    #${flutter}
    ${run}
) 

set_target_properties(FlutterAppJIT PROPERTIES FOLDER "Examples")

target_include_directories(
    FlutterAppJIT
    PRIVATE
)

target_precompile_headers(FlutterAppJIT REUSE_FROM Eureka.Precompiled)



link_library_and_copy_dynamic_library_to_output_path(
    FlutterAppJIT
    flutter_engine_jit
)

target_link_libraries(
    FlutterAppJIT
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




#get_target_property(
#    flutter_engine_imported_location
#    TARGET
#    flutter_engine 
#    PROPERTY
#    IMPORTED_LOCATION
#)
#
#get_target_property(
#    flutter_engine_IMPORTED_IMPLIB
#    TARGET
#    flutter_engine 
#    PROPERTY
#    IMPORTED_IMPLIB
#)
#message("222 flutter_engine IMPORTED_LOCATION = ${flutter_engine_imported_location}")
#message("222 flutter_engine IMPORTED_IMPLIB = ${flutter_engine_IMPORTED_IMPLIB}")