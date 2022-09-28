add_library(flutter_engine SHARED IMPORTED)

#set_target_properties(
#    flutter_engine 
#    PROPERTIES 
#    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/flutter/flutter_engine.dll
#)
#
#set_target_properties(
#    flutter_engine 
#    PROPERTIES 
#    IMPORTED_IMPLIB  ${CMAKE_CURRENT_SOURCE_DIR}/flutter/flutter_engine.dll.lib
#)


#set_source_group(
#    flutter 
#    "flutter_embedding_tests.cpp"
#)
set_source_group(
    vulkan 
    "vulkan_init.tests.cpp"
    "vulkan_memory.tests.cpp"
    "vulkan_objects.tests.cpp"
    "vulkan_pipelines.tests.cpp"
)

set_source_group(
    utils 
    "tests.cpp"
    "formatting.tests.cpp"
    "transform.tests.cpp"
    "fixed_capacity_vector.tests.cpp"
)

set_source_group(
    run 
    "main.cpp"
)
add_executable(
    Eureka.UnitTests
    #${flutter}
    ${vulkan}
    ${utils}
    ${run}
) 

target_include_directories(
    Eureka.UnitTests
    PRIVATE
)

set_compiler_flags(Eureka.UnitTests)   

target_compile_definitions(
   Eureka.UnitTests
   PRIVATE
   CATCH_CONFIG_ENABLE_BENCHMARKING
)

target_precompile_headers(Eureka.UnitTests REUSE_FROM Eureka.Precompiled)

target_link_libraries(
    Eureka.UnitTests
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    #Eureka.Graphics
    Eureka.Shaders
    #Eureka.AssetLoading
	#Eureka.RemoteProto 
	#Eureka.RPC 
	#Eureka.RemoteServer
	#Eureka.RemoteClient
    Catch2::Catch2 
    Catch2::Catch2WithMain
    concurrencpp::concurrencpp
)
