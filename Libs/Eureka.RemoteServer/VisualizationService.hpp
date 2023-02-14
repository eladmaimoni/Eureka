#pragma once

#include <compiler.hpp>
#include <chrono>

#include <deque>
#include <optional>
#include <profiling.hpp>
#include <debugger_trace.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702)
#include <proto/rgorpc.grpc.pb.h>
#include <grpcpp/server_builder.h>
EUREKA_MSVC_WARNING_POP
using namespace std::chrono_literals;

namespace rgo
{
    using GrpcTag = void*;

    struct GRPCCompletion
    {

    };

    class ServerCompletionQueuePollingExecutor
    {
    private:
        std::unique_ptr<grpc::ServerCompletionQueue> _completionQueue;

    public:
        ServerCompletionQueuePollingExecutor(std::unique_ptr<grpc::ServerCompletionQueue> completionQueue)
            : _completionQueue(std::move(completionQueue))
        {

        }

        std::optional<GrpcTag> PollCompletions(std::chrono::system_clock::time_point deadline)
        {
            bool ok = false;
            GrpcTag tag;

            auto nextStatus = _completionQueue->AsyncNext(&tag, &ok , deadline);

            if (nextStatus == grpc::CompletionQueue::GOT_EVENT)
            {
                return tag;
            }
            else if (nextStatus == grpc::CompletionQueue::SHUTDOWN)
            {

            };

            return std::nullopt;
        }

        grpc::ServerCompletionQueue& Get()
        {
            return *_completionQueue;
        }
    };


    enum class CompletionType : uint32_t
    {
        PoseGraphStreaming,
        Something
    };

    struct CompletionTag
    {
        uint32_t       code;
        CompletionType type;
        CompletionTag(uint32_t code, CompletionType type) : code(code), type(type)
        {

        }
        CompletionTag(GrpcTag tag)
        {
            auto value = reinterpret_cast<uint64_t>(tag);
            code = static_cast<uint32_t>(value & 0xFFFF'FFFFu);
            type = static_cast<CompletionType>(value >> 32);
        }

        operator void* () const 
        {
            return reinterpret_cast<void*>(code | static_cast<uint64_t>(type) << 32);
        }
    };

    static_assert(sizeof(CompletionTag) == sizeof(uint64_t));

    enum class HandlerState
    {
        Inactive,
        Listening,
        Finished,
        Streaming
    };

    class PoseGraphStreamingHandler
    {
        static constexpr uint32_t STREAM_CODE = 11;
        static constexpr uint32_t CANCEL_CODE = 22;
        static constexpr uint32_t FINISH_CODE = 33;

        uint32_t                                                         _id{0};
        HandlerState                                                     _state = HandlerState::Inactive;
        eureka::LiveSlamUIService::AsyncService*                     _service;
        grpc::ServerCompletionQueue*                                     _completionQueue;
        eureka::StartPoseGraphUpdatesMsg                                 _clientRequest;
        std::unique_ptr<grpc::ServerContext>                                              _serverContext;
        std::unique_ptr<grpc::ServerAsyncWriter<eureka::PoseGraphVisualizationUpdateMsg>> _asyncWriter;

        //grpc::ServerContext                                              _serverContext;
        //grpc::ServerAsyncWriter<eureka::PoseGraphVisualizationUpdateMsg> _asyncWriter{&_serverContext};

        std::size_t _packetsNum{ 0 };
    public:
        PoseGraphStreamingHandler(eureka::LiveSlamUIService::AsyncService* service, grpc::ServerCompletionQueue* completionQueue)
            : _completionQueue(completionQueue), _service(service)

        {
 
        }

        ~PoseGraphStreamingHandler()
        {
            Stop();
        }

        void Start()
        {
            assert(_state == HandlerState::Inactive);
            _state = HandlerState::Listening;
            Listen();


        }

        void Stop()
        {
            _state = HandlerState::Inactive;
        }

    private:
        void Listen();

    public:
        void Proceed(void* tag, bool ok);
    };

    class VisualizationServer : public std::enable_shared_from_this<VisualizationServer>
    {
        bool                                          _active{ false };
        grpc::ServerBuilder                          _serverBuilder;
        std::unique_ptr<grpc::ServerCompletionQueue> _completionQueue;
        std::unique_ptr<grpc::Server>                _grpcServer;

        eureka::LiveSlamUIService::AsyncService  _service;
        PoseGraphStreamingHandler                    _poseGraphStreamingHandler;

    public:
        VisualizationServer();

        ~VisualizationServer();
        void Start(std::string thisServerListingEndpoint);

        void Stop();



        void PollCompletions(std::chrono::nanoseconds timeout);
        void RunCompletions();

    private:
        void CallHandler(GrpcTag tag, bool ok);

        void ListenToPoseGraphUpdatesRequest()
        {

        }


    };

}

