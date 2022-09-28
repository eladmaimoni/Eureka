#include "RemoteUIServer.hpp"
#include <logging.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <debugger_trace.hpp>

using namespace asio::experimental::awaitable_operators;

namespace eureka
{


    asio::awaitable<void>  RemoteUIServer::ListenToClientRequests()
    {
        for (;;)
        {
            grpc::ServerContext serverContext;
            DoForceUpdateMsg clientRequest;

            grpc::ServerAsyncResponseWriter<GenericResultMsg> responseWriter{ &serverContext };

            

            //auto use_awaitable_token = asio::bind_executor(_completionQueue.Get(), asio::use_awaitable);

            DEBUGGER_TRACE("waiting for client request");

            auto result = co_await agrpc::request(
                &LiveSlamControlCenter::AsyncService::RequestDoForceUpdate,
                _remoteUIService,
                serverContext,
                clientRequest,
                responseWriter,
                asio::use_awaitable
            );

            if (!result)
            {
                DEBUGGER_TRACE("waiting for client request - cancelled");
                co_return;
            }

            DEBUGGER_TRACE("received client request {}", clientRequest.integer());
            GenericResultMsg serverResponse;

            serverResponse.set_integer(clientRequest.integer() + 1);

            co_await agrpc::finish(responseWriter, serverResponse, grpc::Status::OK, asio::use_awaitable);

            DEBUGGER_TRACE("responded to client request");
        }

    }


    asio::awaitable<void> RemoteUIServer::StreamPoseGraph()
    {
        StartPoseGraphUpdatesMsg clientRequest;
        PoseGraphVisualizationUpdateMsg poseGraphUpdate;

        for (;;)
        {
            grpc::ServerContext serverContext;
            grpc::ServerAsyncWriter<PoseGraphVisualizationUpdateMsg> writer{ &serverContext };

            auto initiateStreaming = co_await agrpc::request(
                &LiveSlamControlCenter::AsyncService::RequestStartPoseGraphStreaming,
                _remoteUIService,
                serverContext,
                clientRequest,
                writer,
                asio::use_awaitable
            );

            if (!initiateStreaming)
            {
                DEBUGGER_TRACE("waiting for client stream request - cancelled");
                co_return;
            }

            float val = 1.0f;

            auto writeOk = true;
            std::size_t count = 0;

            while (writeOk && _active)
            {
                std::vector<float> dummy(5, val);

                poseGraphUpdate.poses();
                poseGraphUpdate.mutable_poses()->Assign(dummy.begin(), dummy.end());

                writeOk = co_await agrpc::write(writer, poseGraphUpdate, asio::use_awaitable);

                ++count;

                if (count % 1000 == 0)
                {
                    DEBUGGER_TRACE("server write {}", count);
                }
            }

            auto finishOk = co_await agrpc::finish(writer, grpc::Status::OK, asio::use_awaitable);

            if (!finishOk)
            {
                DEBUGGER_TRACE("could not finish server pose graph streaming, client connection might be interrupted");
            }
            else
            {
                DEBUGGER_TRACE("finished server pose graph streaming");
            }

            DEBUGGER_TRACE("finished server pose graph streaming");
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
            DEBUGGER_TRACE("started server");

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

            DEBUGGER_TRACE("stopped server");
        }
    }



}

