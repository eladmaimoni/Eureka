#include "RemoteUIClient.hpp"
#include <logging.hpp>
#include <asio/detached.hpp>
#include <grpcpp/create_channel.h>

namespace eureka
{

    RemoteUIClient::RemoteUIClient() : _completionQueue(std::make_unique<grpc::CompletionQueue>())
    {

    }

    RemoteUIClient::~RemoteUIClient()
    {
        Stop();
    }

    void RemoteUIClient::Start(std::string remoteServerEndpoint)
    {
        if (!_active)
        {
            //auto channel = grpc::CreateChannel(remoteServerEndpoint, grpc::InsecureChannelCredentials());
            //_completionQueue->Get().clo
            _remoteUIStub = std::make_unique<RemoteUI::Stub>(grpc::CreateChannel(remoteServerEndpoint, grpc::InsecureChannelCredentials()));
            _active = true;
            CLOG("started client");
        }
    }

    void RemoteUIClient::Stop()
    {
        if (_active)
        {
            auto activeStreaming = _activeStreaming.lock();
            if (activeStreaming)
            {
                activeStreaming->TryCancel();
            }
            //_grpcServer->Shutdown();
            //_active = false;

            CLOG("stopped server");
        }
    }

    void RemoteUIClient::SendRequest()
    {
        if (_active)
        {
            asio::co_spawn(_completionQueue.Get(), [this]() { return WriteRequest(); }, asio::detached);
        }
    }

    void RemoteUIClient::RequestPoseGraphUpdates()
    {
        if (_active)
        {
            asio::co_spawn(_completionQueue.Get(), [this]() { return ReadPoseGraphUpdates(); }, asio::detached);
        }
    }

    asio::awaitable<void> RemoteUIClient::WriteRequest()
    {
        using RequestInterface = agrpc::RPC<&RemoteUI::Stub::PrepareAsyncSendRequest>;
        grpc::ClientContext clientContext;
        Request clientRequest;
        clientRequest.set_integer(42);
        Response serverResponse;

        CLOG("sending client request {}", clientRequest.integer());

        auto status = co_await RequestInterface::request(
            _completionQueue.Get(),
            *_remoteUIStub,
            clientContext,
            clientRequest,
            serverResponse,
            asio::use_awaitable
        );

        if (status.ok())
        {
            CLOG("client request sent! response {}", serverResponse.integer());
        }
        else
        {
            CLOG("client request failed");
        }

        co_return;
    }

    asio::awaitable<void> RemoteUIClient::ReadPoseGraphUpdates()
    {
  
        while (_active)
        {
            //grpc::ClientContext clientContext;
            
            auto clientContext = std::make_shared< grpc::ClientContext>();
            _activeStreaming = clientContext;
            VisualizePoseGraph clientRequest;
            clientRequest.set_integer(42);

            std::unique_ptr<grpc::ClientAsyncReader<PoseGraphVisualizationUpdate>> reader;
            CLOG("client requesting pose graph updates");

            auto requestOK = co_await agrpc::request(
                &RemoteUI::Stub::PrepareAsyncServerPoseGraphStreaming,
                *_remoteUIStub,
                *clientContext,
                clientRequest,
                reader,
                asio::use_awaitable
            );

            if (!requestOK)
            {
                CLOG("client requesting pose graph updates - terminated");
                co_return;
            }


            PoseGraphVisualizationUpdate serverResponse;

            uint64_t packetNum = 0;
            for (;;)
            {
                bool readOk = co_await agrpc::read(reader, serverResponse, asio::use_awaitable);

                ++packetNum;
                if (!readOk)
                {
                    CLOG("client requesting pose graph updates - read failed");
                    break;
                }
                else if (packetNum % 10000 == 0)
                {

                    CLOG("got pose graph updates {}", packetNum);
                }
            }

            grpc::Status status;
            auto finishOk = co_await agrpc::finish(reader, status, asio::use_awaitable);

            if (!finishOk)
            {
                CLOG("failed finishing pose graph streaming {}", status.error_message());
            }
            else
            {
                CLOG("finished pose graph streaming {}", status.error_message());
            }
        }
        co_return;

    }

}

