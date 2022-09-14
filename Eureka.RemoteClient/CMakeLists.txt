
set_source_group(
    app 
    "client_main.cpp"
    "RemoteUIClient.hpp"
    "RemoteUIClient.cpp"
)
 
add_executable(
    Eureka.RemoteClient
    ${app}    
) 

set_compiler_flags(Eureka.RemoteClient)   

 target_link_libraries(
     Eureka.RemoteClient 
     PRIVATE
     Eureka.Core
	 Eureka.RPC	 
	 Eureka.RemoteProto
 )
