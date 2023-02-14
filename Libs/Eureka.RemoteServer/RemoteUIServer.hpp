#pragma once
#include <ServerCompletionQueue.hpp>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <grpcpp/server_builder.h>
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP

namespace eureka
{
    class RemoteUIServer
    {
        grpc::ServerBuilder                       _serverBuilder;
        ServerCompletionQueueExecutor             _completionQueue;
        std::unique_ptr<grpc::Server>             _grpcServer;
        rgoproto::LiveSlamUIService::AsyncService _remoteUIService;
        bool                                      _active {false};

        asio::awaitable<void> ListenToClientRequests();
        asio::awaitable<void> StreamPoseGraph();

    public:
        RemoteUIServer();
        ~RemoteUIServer();
        void Start(std::string thisServerListingEndpoint);
        void Stop();
    };

} // namespace eureka
