

add_executable(
    Eureka.UnitTests
    "main.cpp"
    "tests.cpp"
    "formatting.tests.cpp"
    "transform.tests.cpp"
    "fixed_capacity_vector.tests.cpp"
) 

target_include_directories(
    Eureka.UnitTests
    PRIVATE 
    ${TINYGLTF_INCLUDE_DIRS}
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
    Eureka.Types
    Eureka.AssetLoading
    Eureka.Graphics
    Eureka.Shaders
    Catch2::Catch2 
    Catch2::Catch2WithMain
    concurrencpp::concurrencpp
)
