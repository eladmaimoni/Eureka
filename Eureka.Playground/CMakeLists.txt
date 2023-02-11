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
    Eureka.Playground
    #${flutter}
    ${run}
) 

target_include_directories(
    Eureka.Playground
    PRIVATE
)

target_compile_definitions(
   Eureka.Playground
   PRIVATE
   CATCH_CONFIG_ENABLE_BENCHMARKING
)

target_precompile_headers(Eureka.Playground REUSE_FROM Eureka.Precompiled)

function(link_library_and_copy_dynamic_library_to_output_path target_name dynamic_library_target)
    # get the full path to the dll of the dynamic library we wish to link
    get_target_property(
        dynamic_library_target_imported_location # out value
        ${dynamic_library_target}
        IMPORTED_LOCATION
    )

    #message("dynamic_library_target_imported_location = ${dynamic_library_target_imported_location}")

    add_custom_command(
        TARGET 
        ${target_name}  # the target that uses the dll 
        PRE_BUILD       # this command is executed as a pre build step for the target
        COMMAND 
        ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${dynamic_library_target_imported_location}"  # <--this is in-file
        $<TARGET_FILE_DIR:${target_name}>   # <--this is out-file path
    )    
    
    target_link_libraries(
        ${target_name}
        PRIVATE
        ${dynamic_library_target}
    )
endfunction()

link_library_and_copy_dynamic_library_to_output_path(
    Eureka.Playground
    flutter_engine
)

target_link_libraries(
    Eureka.Playground
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
    Eureka.Flutter
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