#pragma once
#include <ClientCompletionQueue.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP
#include <asio/detached.hpp>
#include <debugger_trace.hpp>
#include <logging.hpp>

namespace eureka
{
    template<typename Msg>
    struct StreamingReadRPC
    {
        std::shared_ptr<grpc::ClientContext>          context;
        std::unique_ptr<grpc::ClientAsyncReader<Msg>> reader;
    };


    struct PoseGraphStreamReadPolicy
    {
        using ServiceT = rgoproto::LiveSlamUIService;
        static constexpr auto StreamPrepare = &ServiceT::Stub::PrepareAsyncPoseGraphStreaming;
        using RequestMessage = rgoproto::PoseGraphStreamingRequestMsg;
        using IncomingMessageT = rgoproto::PoseGraphStreamingMsg;

        static RequestMessage MakeRequestMessage()
        {
            RequestMessage clientRequest;
            clientRequest.set_integer(42);
            return clientRequest;
        }
    };

    struct RealtimePoseStreamReadPolicy
    {
        using ServiceT = rgoproto::LiveSlamUIService;
        static constexpr auto StreamPrepare = &ServiceT::Stub::PrepareAsyncRealtimePoseStreaming;
        using RequestMessage = rgoproto::RealtimePoseStreamingRequestMsg;
        using IncomingMessageT = rgoproto::RealtimePoseStreamingMsg;

        static RequestMessage MakeRequestMessage()
        {
            RequestMessage clientRequest;
            clientRequest.set_integer(42);
            return clientRequest;
        }
    };


    template<typename Policy>
    class GenericStreamRead
    {
        using ServiceT = Policy::ServiceT;
        static constexpr auto StreamPrepare = Policy::StreamPrepare;
        using RequestMessage = Policy::RequestMessage;
        using IncomingMessageT = Policy::IncomingMessageT;

        std::shared_ptr<ClientCompletionQueueExecutor>                             _cq;

        std::shared_ptr<grpc::ClientContext>                                       _context;
        bool                                                                       _active{ false };
        std::vector<std::shared_ptr<IncomingMessageT>>                             _messages;
        sigslot::signal<std::shared_ptr<IncomingMessageT>>                         _newMessageSignal;


        std::shared_ptr<IncomingMessageT> GetAvailableMessage()
        {
            // 
            // available message means a message which is owned solely by the stream
            // (and not by other entities)
            // if the shared_ptr reference count is 1, it means we are the only owners
            //
            auto itr = std::ranges::find_if(_messages, [](const auto& msg) { return msg.use_count() == 1; });

            if (itr != _messages.end())
            {
                return *itr;
            }
            else
            {
                return _messages.emplace_back(std::make_shared<IncomingMessageT>());
            }
        }

        asio::awaitable_optional<StreamingReadRPC<IncomingMessageT>> DoInitiateReadStream(ServiceT::Stub& stub)
        {
            StreamingReadRPC<IncomingMessageT> rpc;
            rpc.context = std::make_shared<grpc::ClientContext>();

            _context = rpc.context;


            RequestMessage clientRequest = Policy::MakeRequestMessage();
           

            bool requestOK = false;

            //
            // 
            //
            std::tie(rpc.reader, requestOK) = co_await agrpc::request(
                StreamPrepare,
                stub,
                *rpc.context,
                clientRequest,
                _cq->GetCompletionToken()
            );

            if (requestOK)
            {
                co_return rpc;
            }
            else
            {
                grpc::Status status;
                /*auto bb =*/ co_await agrpc::finish(rpc.reader, status);


                //DEBUGGER_TRACE("request failed {} ", status.error_message());
                _context = nullptr;
            }

            co_return std::nullopt;
        }

        asio::awaitable<void> DoReadStream(std::shared_ptr<typename ServiceT::Stub> stub)
        {
            if (_active)
            {
                co_return;
            }

            _active = true;

            while (_active)
            {
                std::optional<StreamingReadRPC<IncomingMessageT>> activeRPC;
                while (_active)
                {
                    //DEBUGGER_TRACE("client requesting pose graph streaming");
                    activeRPC = co_await DoInitiateReadStream(*stub);

                    if (activeRPC)
                    {
                        break;
                    }
                    else
                    {
                        grpc::Alarm alarm;
                        co_await agrpc::wait(alarm, grpc_deadline_from_now(500ms));
                    }
                }

                if (!activeRPC)
                {
                    // we broke because _poseGraphStreamingActive == false
                    assert(_active == false);
                    DEBUGGER_TRACE("client requesting pose graph streaming - cancelled");
                    co_return;
                }

                auto [clientContext, reader] = std::move(*activeRPC);



                uint64_t packetNum = 0;

                while (_active)
                {
                    auto msg = GetAvailableMessage();

                    bool readOk = co_await agrpc::read(reader, *msg, _cq->GetCompletionToken());

                    ++packetNum;
                    if (!readOk)
                    {
                         DEBUGGER_TRACE("client requesting pose graph updates - read failed");
                        break;
                    }
                    else if (packetNum % 10000 == 0)
                    {

                        DEBUGGER_TRACE("got pose graph updates {}", packetNum);
                    }

                    _newMessageSignal(std::move(msg));
                }


                grpc::Status status;
                auto finishOk = co_await agrpc::finish(reader, status, _cq->GetCompletionToken());
                if (!finishOk)
                {
                    DEBUGGER_TRACE("failed finishing pose graph streaming {} total packets read {}", status.error_message(), packetNum);
                    CLOG("failed finishing pose graph streaming {} total packets read {}", status.error_message(), packetNum);
                }
                else
                {
                    DEBUGGER_TRACE("finished pose graph streaming {} total packets read {}", status.error_message(), packetNum);
                    CLOG("finished pose graph streaming {} total packets read {}", status.error_message(), packetNum);
                }

                _context.reset();

            }

            //_poseGraphStreamingContext.reset();
            _active = false;
            DEBUGGER_TRACE("client streaming inactive");
            co_return;
        }

        void DoCancelStream()
        {
            _active = false;
            if (_context)
            {
                _context->TryCancel();

                DEBUGGER_TRACE("client - tried cancelling active pose graph streaming");
                CLOG("client - tried cancelling active pose graph streaming");
                _context.reset();
            }
        }
    public:
        GenericStreamRead(std::shared_ptr<ClientCompletionQueueExecutor> cq)
            : _cq(std::move(cq))
        {

        }

        bool IsActive() const
        {
            return _active;
        }

        void Start(std::shared_ptr<typename ServiceT::Stub> stub)
        {
            asio::co_spawn(_cq->Get(),
                [this, stub = std::move(stub)]() mutable -> asio::awaitable<void>
                {
                    co_await DoReadStream(std::move(stub));
                    co_return;
                },
                asio::detached
            );
        }

        void Stop()
        {
            asio::co_spawn(_cq->Get(),
                [this]() -> asio::awaitable<void>
                {
                    DoCancelStream();

                    co_return;
                },
                asio::detached
                    );
        }

        template<typename Callable>
        sigslot::connection ConnectSlot(Callable&& slot)
        {
            return _newMessageSignal.connect(std::forward<Callable>(slot));
        }
    };


    using PoseGraphStreamRead = GenericStreamRead<PoseGraphStreamReadPolicy>;
    using RealtimePoseStreamRead = GenericStreamRead<RealtimePoseStreamReadPolicy>;
}

