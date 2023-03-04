
set_source_group(
    flutter 
    "FlutterUtils.hpp"
    "FlutterUtils.cpp"
    "VulkanDesktopEmbedder.hpp"
    "VulkanDesktopEmbedder.cpp"
    "TaskRunners.hpp"
    "TaskRunners.cpp"
    "VulkanCompositor.hpp"
    "VulkanCompositor.cpp"

)


add_library(
    Eureka.Flutter
    STATIC
    ${flutter}
 )
 set_target_properties(Eureka.Flutter PROPERTIES FOLDER "Libs")
 configure_code_analysis(Eureka.Flutter)
target_include_directories(
    Eureka.Flutter
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    Eureka.Flutter 
    PUBLIC
    Eureka.Core 
    Eureka.Graphics 
	Eureka.Windowing
    Eureka.Shaders 
    flutter_engine    
    PRIVATE
    eureka_strict_compiler_flags
)

