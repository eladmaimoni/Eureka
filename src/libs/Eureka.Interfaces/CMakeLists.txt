

set_source_group(
	ui
	"IImGuiLayout.hpp"
	"IImGuiLayout.cpp"
)

add_library(
	Eureka.Interfaces 
	STATIC
	${ui}
) 


set_target_properties(Eureka.Interfaces PROPERTIES FOLDER "Libs")

configure_code_analysis(Eureka.Interfaces)

target_include_directories(
    Eureka.Interfaces
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
	Eureka.Interfaces
    PRIVATE 
	eureka_strict_compiler_flags
)
