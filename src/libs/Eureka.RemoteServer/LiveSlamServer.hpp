#include "GrpcContext.hpp"




namespace eureka::rpc
{
    class LiveSlamServer
    {
        grpc::ServerBuilder                          _builder;
        std::vector<std::shared_ptr<grpc::Service>>  _services; // services must outlive the server
        std::unique_ptr<grpc::Server>                _grpcServer;
        std::shared_ptr<GrpcContext>                 _grpcContext;
        bool                                         _active{ false };
    public:
        LiveSlamServer(std::vector<std::shared_ptr<grpc::Service>> services);
        ~LiveSlamServer();

        std::shared_ptr<GrpcContext> GetContext();
        void Start(std::string thisServerListingEndpoint);
        void Shutdown();
    };

    

}

