

set_source_group(
    app 
    "server_main.cpp"
    "RemoteUIServer.hpp"
    "RemoteUIServer.cpp"
)

add_executable(
    Eureka.RemoteServer
    ${app}    
) 
 
set_compiler_flags(Eureka.RemoteServer)   

 target_link_libraries(
     Eureka.RemoteServer 
     PRIVATE
     Eureka.RPC
	 Eureka.RemoteProto
 )
  