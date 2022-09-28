#pragma once
#include <ClientCompletionQueue.hpp>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <proto/eureka.grpc.pb.h>
EUREKA_MSVC_WARNING_POP

#include "PoseGraphStreamer.hpp"

namespace eureka
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
        std::shared_ptr<LiveSlamControlCenter::Stub>                                _remoteLiveSlamStub;
        std::atomic<ConnectionState>                                                _state{ ConnectionState::Disconnected };
        std::stop_source                                                            _connectCancellationSource;

        PoseGraphStreamRead _poseGraphStreamRead;

        asio::awaitable<void> DoDisconnect();
        asio::awaitable<void> DoWaitForConnection(std::stop_token stopToken, std::shared_ptr <promise_t<void>> pendingConnection);
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
        future_t<void> ConnectAsync(std::string remoteServerEndpoint);
        void CancelConnecting();
        void Disconnect();
        ConnectionState GetConnectionState() const { return _state; }
        
        //
        // Pose Graph Streaming
        //

        void StartReadingPoseGraphUpdates();
        void StopReadingPoseGraphUpdates();
        template<typename Callable>
        sigslot::connection ConnectPoseGraphSlot(Callable&& slot)
        {
            return _poseGraphStreamRead.ConnectSlot(std::move(slot));
        }


    };


}

