

set_source_group(
	ui
	"IImGuiLayout.hpp"
	"IImGuiLayout.cpp"
)

add_library(
	Eureka.Interfaces 
	${ui}
) 
 
set_compiler_flags(Eureka.Interfaces) 

target_include_directories(
    Eureka.Interfaces
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

