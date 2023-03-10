

set_source_group(
    src 
    GrpcContext.hpp
    GrpcContext.cpp
    LiveSlamServer.hpp
    LiveSlamServer.cpp
    LiveSlamServiceHelpers.hpp
    LiveSlamServiceHelpers.cpp
    RemoteUIServer.hpp
    RemoteUIServer.cpp
    StreamHandlers.hpp
    StreamHandlers.cpp
    UnaryHandlers.hpp
	VisualizationService.hpp
    VisualizationService.cpp
)

add_library(
    Eureka.RemoteServer
    ${src}    
) 
 
target_include_directories(
    Eureka.RemoteServer
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(Eureka.RemoteServer PROPERTIES FOLDER "Libs")

target_link_libraries(
     Eureka.RemoteServer 
     PUBLIC
     Eureka.RPC
	 Eureka.RemoteProto
     PRIVATE
     eureka_strict_compiler_flags
 )
  