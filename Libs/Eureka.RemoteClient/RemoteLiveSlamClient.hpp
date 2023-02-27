#pragma once
#include <ClientCompletionQueue.hpp>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702 4005)
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP
#include <stop_token.hpp>
#include "PoseGraphStreamer.hpp"

namespace eureka::rpc
{
    enum class ConnectionState
    {
        Disconnected,
        Connected,
        Disconnecting,
        Connecting
    };

    class RemoteLiveSlamClient
    {
        // as long as we use a single thread (Main) to access this object, no need for a mutex
        //std::mutex                                       _mtx; 
        std::shared_ptr<ClientCompletionQueuePollingExecutor>                       _completionQueue;

        std::shared_ptr<grpc::Channel>                                              _channel;
        std::shared_ptr<rgoproto::LiveSlamUIService::Stub>                            _remoteLiveSlamStub;
        std::atomic<ConnectionState>                                                _state{ ConnectionState::Disconnected };
        stop_source                                                                _connectCancellationSource;
        sigslot::signal<ConnectionState>                                            _connectionStateSignal;

        PoseGraphStreamRead _poseGraphStreamRead;
        RealtimePoseStreamRead _realtimePoseStreamRead;

        asio::awaitable<void> DoDisconnect();
        asio::awaitable<void> DoMonitorConnection(stop_token stopToken);
        asio::awaitable<void> DoWaitForConnection(stop_token stopToken);

        void SetConnectionState(ConnectionState state);
    public:
        RemoteLiveSlamClient();
        ~RemoteLiveSlamClient();

        //
        // Completions
        //
        void PollCompletions(std::chrono::nanoseconds duration);
        void PollCompletions();
        void RunCompletions();
        
        //
        // Connect
        //
        void ConnectAsync(std::string remoteServerEndpoint);
        void CancelConnecting();
        void Disconnect();
        ConnectionState GetConnectionState() const { return _state; }
        
        //
        // Pose Graph Streaming
        //

        void StartStreams();
        void StopStreams();
        void SendForceGPOOptimization();
        template<typename Callable>
        sigslot::connection ConnectConnectionStateSlot(Callable&& slot)
        {
            return _connectionStateSignal.connect(std::move(slot));
        }

        template<typename Callable>
        sigslot::connection ConnectPoseGraphSlot(Callable&& slot)
        {
            return _poseGraphStreamRead.ConnectSlot(std::move(slot));
        }
        template<typename Callable>
        sigslot::connection ConnectRealtimePoseSlot(Callable&& slot)
        {
            return _realtimePoseStreamRead.ConnectSlot(std::move(slot));
        }

    };


}

