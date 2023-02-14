#pragma once
#include <ClientCompletionQueue.hpp>
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <proto/eureka.grpc.pb.h>
EUREKA_MSVC_WARNING_POP


namespace asio
{
    template<typename T>
    using awaitable_optional = asio::awaitable<std::optional<T>>;
}

namespace eureka
{
    template<typename Msg>
    struct StreamingReadRPC
    {
        std::shared_ptr<grpc::ClientContext>          context;
        std::unique_ptr<grpc::ClientAsyncReader<Msg>> reader;
    };

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
        ClientCompletionQueuePollingExecutor                                _completionQueue;

        std::shared_ptr<grpc::Channel>                                      _channel;
        std::shared_ptr<LiveSlamControlCenter::Stub>                        _remoteLiveSlamStub;
        std::atomic<ConnectionState>                                        _state{ ConnectionState::Disconnected };
        std::stop_source                                                    _connectCancellationSource;

        std::shared_ptr<grpc::ClientContext>                                       _poseGraphStreamingContext;
        bool                                                                       _poseGraphStreamingActive{ false };
        std::vector<std::shared_ptr<PoseGraphVisualizationUpdateMsg>>              _poseGraphMessages;
                                                                                 
        sigslot::signal<std::shared_ptr<PoseGraphVisualizationUpdateMsg>>          _newPoseGraphSignal;

        asio::awaitable<void> DoWaitForConnection(std::stop_token stopToken, std::shared_ptr <promise_t<void>> pendingConnection);
        asio::awaitable<void> DoDisconnect();


        asio::awaitable<void> DoReadPoseGraphUpdateStream();
        void DoCancelPoseGraphStreaming();

        asio::awaitable_optional<StreamingReadRPC<PoseGraphVisualizationUpdateMsg>> DoInitiatePoseGraphUpdateStream(LiveSlamControlCenter::Stub& stub);

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
        std::shared_ptr<PoseGraphVisualizationUpdateMsg> GetAvailableMessage();
        void StartReadingPoseGraphUpdates();
        void StopReadingPoseGraphUpdates();


     
        template<typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            return _newPoseGraphSignal.connect(std::forward<Callable>(slot));
        }


    };


}

