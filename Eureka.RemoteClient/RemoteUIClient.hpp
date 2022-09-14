#pragma once
#include <ClientCompletionQueue.hpp>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <eureka.grpc.pb.h>
EUREKA_MSVC_WARNING_POP

namespace eureka
{


    class RemoteUIClient
    {
        ClientCompletionQueueExecutor     _completionQueue;
        std::unique_ptr<RemoteUI::Stub>   _remoteUIStub;
        std::atomic_bool                  _active{ false };
        std::weak_ptr<grpc::ClientContext> _activeStreaming;

        asio::awaitable<void> ReadPoseGraphUpdates();
        asio::awaitable<void> WriteRequest();

    public:
        RemoteUIClient();
        ~RemoteUIClient();

        void SendRequest();
        void RequestPoseGraphUpdates();
        void Start(std::string remoteServerEndpoint);
        void Stop();


    };


}

