

set_source_group(asset "tingltf.cpp")

add_library(
    Eureka.AssetLoading     
    ${asset}
)

set_compiler_flags(Eureka.AssetLoading) 

target_precompile_headers(Eureka.AssetLoading REUSE_FROM Eureka.Precompiled)

target_include_directories(
    Eureka.AssetLoading
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE 
    ${TINYGLTF_INCLUDE_DIRS}
)

target_link_libraries(
    Eureka.AssetLoading
    PRIVATE
    Eureka.Core 
    Eureka.Types 
    Eureka.Graphics 
    concurrencpp::concurrencpp
    Eureka.Precompiled 
)