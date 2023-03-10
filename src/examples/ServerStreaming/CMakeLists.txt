add_executable(
	ServerStreaming
	server_main.cpp
	)

set_target_properties(ServerStreaming PROPERTIES FOLDER "Examples")

target_link_libraries(
    ServerStreaming
    PRIVATE 
    Eureka.Precompiled
    Eureka.RemoteServer

    eureka_strict_compiler_flags
    #concurrencpp::concurrencpp
)