


set_source_group(
	async 
	"ClientCompletionQueue.hpp" 
	"ClientCompletionQueue.cpp"
	"ServerCompletionQueue.hpp" 
	"ServerCompletionQueue.cpp"
	"UnifiedCompletionQueue.hpp" 
	"UnifiedCompletionQueue.cpp"
)

add_library(
	Eureka.RPC 
	${async}
) 
set_target_properties(Eureka.RPC PROPERTIES FOLDER "Libs")
set_compiler_flags(Eureka.RPC) 

target_include_directories(
    Eureka.RPC
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
     Eureka.RPC 
     PUBLIC
	 Eureka.Core
	 gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts
     asio-grpc::asio-grpc-standalone-asio     
)
