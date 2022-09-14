#include "RemoteUIServer.hpp"
#include <logging.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;

namespace eureka
{


    asio::awaitable<void> RemoteUIServer::ListenToClientRequests()
    {
        for (;;)
        {
            grpc::ServerContext serverContext;
            Request clientRequest;

            grpc::ServerAsyncResponseWriter<Response> responseWriter{ &serverContext };

            //auto use_awaitable_token = asio::bind_executor(_completionQueue.Get(), asio::use_awaitable);

            CLOG("waiting for client request");

            auto result = co_await agrpc::request(
                &RemoteUI::AsyncService::RequestSendRequest,
                _remoteUIService,
                serverContext,
                clientRequest,
                responseWriter,
                asio::use_awaitable
            );

            if (!result)
            {
                CLOG("waiting for client request - cancelled");
                co_return;
            }

            CLOG("received client request {}", clientRequest.integer());
            Response serverResponse;

            serverResponse.set_integer(clientRequest.integer() + 1);

            co_await agrpc::finish(responseWriter, serverResponse, grpc::Status::OK, asio::use_awaitable);

            CLOG("responded to client request");
        }

    }


    asio::awaitable<void> RemoteUIServer::StreamPoseGraph()
    {
        VisualizePoseGraph clientRequest;
        PoseGraphVisualizationUpdate poseGraphUpdate;

        for (;;)
        {
            grpc::ServerContext serverContext;
            grpc::ServerAsyncWriter<PoseGraphVisualizationUpdate> writer{ &serverContext };

            auto initiateStreaming = co_await agrpc::request(
                &RemoteUI::AsyncService::RequestServerPoseGraphStreaming,
                _remoteUIService,
                serverContext,
                clientRequest,
                writer,
                asio::use_awaitable
            );

            if (!initiateStreaming)
            {
                CLOG("waiting for client stream request - cancelled");
                co_return;
            }

            float val = 1.0f;

            auto writeOk = true;

            while (writeOk && _active)
            {
                std::vector<float> dummy(5, val);

                poseGraphUpdate.poses();
                poseGraphUpdate.mutable_poses()->Assign(dummy.begin(), dummy.end());

                writeOk = co_await agrpc::write(writer, poseGraphUpdate, asio::use_awaitable);
            }

            auto finishOk = co_await agrpc::finish(writer, grpc::Status::OK, asio::use_awaitable);

            if (!finishOk)
            {
                CLOG("could not finish server pose graph streaming, client connection might be interrupted");
            }
            else
            {
                CLOG("finished server pose graph streaming");
            }

            CLOG("finished server pose graph streaming");
        }

    }

    RemoteUIServer::RemoteUIServer() : _completionQueue(_serverBuilder.AddCompletionQueue())
    {

    }

    RemoteUIServer::~RemoteUIServer()
    {
        Stop();
    }

    void RemoteUIServer::Start(std::string thisServerListingEndpoint)
    {
        if (!_active)
        {
            _serverBuilder.AddListeningPort(thisServerListingEndpoint, grpc::InsecureServerCredentials());
            _serverBuilder.RegisterService(&_remoteUIService);
            _grpcServer = _serverBuilder.BuildAndStart();
            _active = true;
            CLOG("started server");

            asio::co_spawn(
                _completionQueue.Get(),
                [this]() -> asio::awaitable<void>
                {
                    // The two operations below will run concurrently on the same thread.              
                    co_await (ListenToClientRequests() || StreamPoseGraph());
                },
                asio::detached
            );
        }
    }

    void RemoteUIServer::Stop()
    {
        if (_active)
        {
            _grpcServer->Shutdown();
            _active = false;

            CLOG("stopped server");
        }
    }



}

