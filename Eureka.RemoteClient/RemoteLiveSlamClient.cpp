#include "RemoteLiveSlamClient.hpp"
#include <logging.hpp>
#include <asio/detached.hpp>
#include <grpcpp/create_channel.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>

using namespace std::chrono_literals;

namespace eureka
{
    RemoteLiveSlamClient::RemoteLiveSlamClient()
        :
        _completionQueue(std::make_shared<ClientCompletionQueuePollingExecutor>()),
        _poseGraphStreamRead(_completionQueue)
    {

    }

    RemoteLiveSlamClient::~RemoteLiveSlamClient()
    {
        _poseGraphStreamRead.Stop();

        CancelConnecting();
        Disconnect();

        while (_poseGraphStreamRead.IsActive())
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

    future_t<void> RemoteLiveSlamClient::ConnectAsync(std::string remoteServerIp)
    {
        const auto server_grpc_port = ":50051"; 
        const auto remoteServerEndpoint = remoteServerIp + server_grpc_port;

        auto expected = ConnectionState::Disconnected;
        if (_state.compare_exchange_strong(expected, ConnectionState::Connecting))
        {
            auto pendingConnection = std::make_shared<promise_t<void>>();
            _connectCancellationSource = std::stop_source();
            auto stopToken = _connectCancellationSource.get_token();

            auto fut = pendingConnection->get_result();
            _channel = grpc::CreateChannel(remoteServerEndpoint, grpc::InsecureChannelCredentials());
           
            asio::co_spawn(_completionQueue->Get(),
                [this, pendingConnection = std::move(pendingConnection), stopToken = std::move(stopToken)]() -> asio::awaitable<void>
                {
                    co_await DoWaitForConnection(std::move(stopToken), std::move(pendingConnection));
                },
                asio::detached
                );

            return fut;
        }
        else
        {
            return concurrencpp::make_exceptional_result<void>(std::make_exception_ptr(std::logic_error("busy connecting or connected")));
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



    void RemoteLiveSlamClient::StartReadingPoseGraphUpdates()
    {
        auto stub = _remoteLiveSlamStub;
        if (stub)
        {
            _poseGraphStreamRead.Start(std::move(stub));
        }
      
    }

    void RemoteLiveSlamClient::StopReadingPoseGraphUpdates()
    {
        _poseGraphStreamRead.Stop();
    }

    asio::awaitable<void> RemoteLiveSlamClient::DoWaitForConnection(std::stop_token stopToken, std::shared_ptr <promise_t<void>> pendingConnection)
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
            _remoteLiveSlamStub = std::make_unique<LiveSlamControlCenter::Stub>(_channel);
            pendingConnection->set_result();
            _state.store(ConnectionState::Connected);

        }
        else
        {
            DEBUGGER_TRACE("channel failed to connect. state = {}", to_string(state));
            pendingConnection->set_exception(std::make_exception_ptr(connection_failed("connection failed")));

            _channel.reset();
            _state = ConnectionState::Disconnected;
        }



        co_return;
    }

    asio::awaitable<void> RemoteLiveSlamClient::DoDisconnect()
    {
        _channel.reset();
        _remoteLiveSlamStub.reset();
        _state = ConnectionState::Disconnected;
        co_return;
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

//asio::awaitable<void> RemoteLiveSlamClient::WriteRequest()
//{
//    using RequestInterface = agrpc::RPC<&LiveSlamControlCenter::Stub::PrepareAsyncDoForceUpdate>;
//    grpc::ClientContext clientContext;
//    DoForceUpdateMsg clientRequest;
//    clientRequest.set_integer(42);
//    GenericResultMsg serverResponse;

//    DEBUGGER_TRACE("sending client request {}", clientRequest.integer());

//    auto status = co_await RequestInterface::request(
//        _completionQueue.Get(),
//        *_remoteLiveSlamStub,
//        clientContext,
//        clientRequest,
//        serverResponse,
//        _completionQueue.GetCompletionToken()
//    );

//    if (status.ok())
//    {
//        DEBUGGER_TRACE("client request sent! response {}", serverResponse.integer());
//    }
//    else
//    {
//        DEBUGGER_TRACE("client request failed");
//    }

//    co_return;
//}
