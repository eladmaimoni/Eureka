#include "RemoteLiveSlamClient.hpp"
#include <logging.hpp>
#include <asio/detached.hpp>
#include <grpcpp/create_channel.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>

using namespace std::chrono_literals;

namespace eureka::rpc
{
    RemoteLiveSlamClient::RemoteLiveSlamClient()
        :
        _completionQueue(std::make_shared<ClientCompletionQueuePollingExecutor>()),
        _poseGraphStreamRead(_completionQueue),
        _realtimePoseStreamRead(_completionQueue)
    {

    }

    RemoteLiveSlamClient::~RemoteLiveSlamClient()
    {
        _poseGraphStreamRead.Stop();
        _realtimePoseStreamRead.Stop();

        CancelConnecting();
        Disconnect();

        while (_poseGraphStreamRead.IsActive())
        {
            _completionQueue->PollCompletions();
        }
        while (_realtimePoseStreamRead.IsActive())
        {
            _completionQueue->PollCompletions();
        }

        while (_state != ConnectionState::Disconnected)
        {
            _completionQueue->PollCompletions();
        }
        //_remoteLiveSlamStub.reset();
        //DEBUGGER_TRACE("stub deleted");
    }

    void RemoteLiveSlamClient::ConnectAsync(std::string remoteServerIp)
    {
        const auto server_grpc_port = ":50051"; 
        const auto remoteServerEndpoint = remoteServerIp + server_grpc_port;

        auto expected = ConnectionState::Disconnected;
        if (_state.compare_exchange_strong(expected, ConnectionState::Connecting))
        {
            _connectCancellationSource = std::stop_source();
            auto stopToken = _connectCancellationSource.get_token();

            _channel = grpc::CreateChannel(remoteServerEndpoint, grpc::InsecureChannelCredentials());
           
            asio::co_spawn(_completionQueue->Get(),
                [this, stopToken = std::move(stopToken)]() -> asio::awaitable<void>
                {
                    co_await DoWaitForConnection(std::move(stopToken));
                },
                asio::detached
                );
        }
        else
        {
            throw std::logic_error("busy connecting or connected");
        }
    }

    void RemoteLiveSlamClient::CancelConnecting()
    {
        _connectCancellationSource.request_stop();
    }

    void RemoteLiveSlamClient::Disconnect()
    {
        auto expected = ConnectionState::Connected;
        if (_state.compare_exchange_strong(expected, ConnectionState::Disconnecting))
        {
            asio::co_spawn(_completionQueue->Get(),
                [this]() -> asio::awaitable<void>
                {
                    co_await DoDisconnect();
                    co_return;
                },
                asio::detached
            );     
            DEBUGGER_TRACE("stopped client");
        }
    }

    void RemoteLiveSlamClient::StartStreams()
    {
        auto stub = _remoteLiveSlamStub;
        if (stub)
        {
            _poseGraphStreamRead.Start(stub);
            _realtimePoseStreamRead.Start(std::move(stub));
        }      
    }

    void RemoteLiveSlamClient::StopStreams()
    {
        _poseGraphStreamRead.Stop();
        _realtimePoseStreamRead.Stop();
    }

    asio::awaitable<void> RemoteLiveSlamClient::DoWaitForConnection(std::stop_token stopToken)
    {
        auto state = _channel->GetState(true); // true means initiate connection

        DEBUGGER_TRACE("channel - trying to connect, state = {}", to_string(state));
        //pendingConnection->set_result();
        while (state != grpc_connectivity_state::GRPC_CHANNEL_READY && !stopToken.stop_requested())
        {
            auto deadline = std::chrono::system_clock::now() + 500ms;

            /*auto deadlineNotExpired = */co_await agrpc::grpc_initiate([this, state, deadline](agrpc::GrpcContext& grpc_context, void* tag)
                {
                    _channel->NotifyOnStateChange(state, deadline, agrpc::get_completion_queue(grpc_context), tag);
                },
                asio::use_awaitable
                    );

            state = _channel->GetState(false);

            //DEBUGGER_TRACE("channel state = {}", to_string(state));
        }
        DEBUGGER_TRACE("channel state checking over");
        if (state == grpc_connectivity_state::GRPC_CHANNEL_READY)
        {
            DEBUGGER_TRACE("channel ready, state = {}", to_string(state));
            _remoteLiveSlamStub = std::make_unique<rgoproto::LiveSlamUIService::Stub>(_channel);

            SetConnectionState(ConnectionState::Connected);

            asio::co_spawn(_completionQueue->Get(),
                [this, stopToken = std::move(stopToken)]() -> asio::awaitable<void>
                {
                    co_await DoMonitorConnection(std::move(stopToken));
                },
                asio::detached
            );        
        }
        else
        {
            DEBUGGER_TRACE("channel failed to connect. state = {}", to_string(state));


            _channel.reset();

            SetConnectionState(ConnectionState::Disconnected);
        }

        co_return;
    }

    void RemoteLiveSlamClient::SetConnectionState(ConnectionState state)
    {
        _state.store(state);
        _connectionStateSignal(state);
    }

    asio::awaitable<void> RemoteLiveSlamClient::DoDisconnect()
    {
        _channel.reset();
        _remoteLiveSlamStub.reset();
        SetConnectionState(ConnectionState::Disconnected);
        
        co_return;
    }

    asio::awaitable<void> RemoteLiveSlamClient::DoMonitorConnection(std::stop_token stopToken)
    {
        auto state = _channel->GetState(false); // true means initiate connection

        DEBUGGER_TRACE("channel - trying to connect, state = {}", to_string(state));

        while (state == grpc_connectivity_state::GRPC_CHANNEL_READY)
        {
            auto channel = _channel; // can be destroyed while we are waiting

            if (!channel) break;


            auto deadline = std::chrono::system_clock::now() + 500ms;

            co_await agrpc::grpc_initiate([this, channel, state, deadline](agrpc::GrpcContext& grpc_context, void* tag)
                {
                    channel->NotifyOnStateChange(state, deadline, agrpc::get_completion_queue(grpc_context), tag);
                },
                asio::use_awaitable
                    );

            state = channel->GetState(false);

            if (state != grpc_connectivity_state::GRPC_CHANNEL_READY)
            {
                SetConnectionState(ConnectionState::Connecting);

                asio::co_spawn(_completionQueue->Get(),
                    [this, stopToken = std::move(stopToken)]() -> asio::awaitable<void>
                    {
                        co_await DoWaitForConnection(std::move(stopToken));
                    },
                    asio::detached
                );
            }

            //DEBUGGER_TRACE("channel state = {}", to_string(state));
        }
    }

    void RemoteLiveSlamClient::SendForceGPOOptimization()
    {
        asio::co_spawn(_completionQueue->Get(),
            [this]() -> asio::awaitable<void>
            {
                auto stub = _remoteLiveSlamStub;

                if (stub)
                {
                    rgoproto::ForceFullGPORequestMsg clientRequest{};
                    grpc::ClientContext clientContext{};
                    //auto methodPtr = &rgoproto::LiveSlamUIService::Stub::AsyncForceFullGPO;
                    //agrpc::GrpcContext* ctx = nullptr;

                    //auto& ctx1 = _completionQueue->Get();
                    auto ctx2 = _completionQueue->GetCompletionToken();
                    //_completionQueue->GetCompletionToken();
                    auto reader = agrpc::request(
                        &rgoproto::LiveSlamUIService::Stub::AsyncForceFullGPO,
                        *stub,
                        clientContext,
                        clientRequest,
                        _completionQueue->Get()
                        //_completionQueue->GetCompletionToken() // why not working ???
                    );

                    co_await agrpc::read_initial_metadata(reader, asio::use_awaitable);

                    rgoproto::ForceFullGPOResponseMsg clientResponse{};
                    grpc::Status status;
                    co_await agrpc::finish(reader, clientResponse, status, asio::use_awaitable);
                }

                co_return;
            },
            asio::detached
                );

    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                             PollCompletions
    //
    //////////////////////////////////////////////////////////////////////////

    void RemoteLiveSlamClient::PollCompletions()
    {
        _completionQueue->PollCompletions();
    }

    void RemoteLiveSlamClient::PollCompletions(std::chrono::nanoseconds duration)
    {
        _completionQueue->PollCompletions(duration);
    }


    void RemoteLiveSlamClient::RunCompletions()
    {
        _completionQueue->PollCompletions();
    }


}

