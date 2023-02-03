
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
    ${vulkan}
    ${utils}
    ${run}
) 

target_include_directories(
    Eureka.UnitTests
    PRIVATE
)

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
    Eureka.Graphics
    Eureka.Shaders
    Eureka.Flutter
    #Eureka.AssetLoading
	#Eureka.RemoteProto 
	#Eureka.RPC 
	#Eureka.RemoteServer
	#Eureka.RemoteClient
    Catch2::Catch2 
    eureka_strict_compiler_flags
    #concurrencpp::concurrencpp
)
