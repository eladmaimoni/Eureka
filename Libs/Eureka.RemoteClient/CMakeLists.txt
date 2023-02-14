
set_source_group(
    ui 
    "RemoteUIModel.hpp"
    "RemoteUIModel.cpp"
    "RemoteLiveSlamUI.hpp"
    "RemoteLiveSlamUI.cpp"
)

set_source_group(
    rpc 
    "PoseGraphStreamer.hpp"
    "PoseGraphStreamer.cpp"
    "RemoteLiveSlamClient.hpp"
    "RemoteLiveSlamClient.cpp"
)

add_library(
    Eureka.RemoteClient
    ${ui}   
    ${rpc}
) 

target_include_directories(
    Eureka.RemoteClient
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(
    Eureka.RemoteClient 
    REUSE_FROM
    Eureka.Precompiled
)

set_compiler_flags(Eureka.RemoteClient)   
set_target_properties(Eureka.RemoteClient PROPERTIES FOLDER "Libs")

 target_link_libraries(
     Eureka.RemoteClient 
     PRIVATE
     Eureka.Precompiled
     Eureka.Core
     Eureka.Serialization
     Eureka.Interfaces
	 Eureka.RPC	 
	 Eureka.RemoteProto
     implot::implot
 )
