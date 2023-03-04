

set_source_group(
    app 
    "server_main.cpp"
    "RemoteUIServer.hpp"
    "RemoteUIServer.cpp"
	"VisualizationService.hpp"
    "VisualizationService.cpp"
)

add_library(
    Eureka.RemoteServer
    ${app}    
) 
 
target_include_directories(
    Eureka.RemoteServer
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)


set_compiler_flags(Eureka.RemoteServer)   

 target_link_libraries(
     Eureka.RemoteServer 
     PRIVATE
     Eureka.RPC
	 Eureka.RemoteProto
 )
  