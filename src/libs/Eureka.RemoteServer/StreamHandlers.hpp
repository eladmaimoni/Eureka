#pragma once
#include "LiveSlamServiceHelpers.hpp"
#include "GrpcContext.hpp"
#include <memory>

EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702)
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP

namespace eureka::rpc
{
    enum class HandlerState
    {
        Inactive,
        Listening,
        Finished,
        Cancelled,
        Stopping,
        WaitingForAvailableData,
        WaitngForWriteDone
    };

    struct GPOStreamPolicy
    {
        using StreamRequestMsg = rgoproto::PoseGraphStreamingRequestMsg;
        using StreamMsg = rgoproto::PoseGraphStreamingMsg;
        using AsyncService = rgoproto::LiveSlamUIService::AsyncService;
        using StreamMethodT = decltype(&rgoproto::LiveSlamUIService::AsyncService::RequestPoseGraphStreaming);

        static constexpr StreamMethodT StreamRequestMethod = &rgoproto::LiveSlamUIService::AsyncService::RequestPoseGraphStreaming;

        static constexpr char PRETTY_NAME[] = "GPO Stream";
    };

    struct RealtimePoseStreamPolicy
    {
        using StreamRequestMsg = rgoproto::RealtimePoseStreamingRequestMsg;
        using StreamMsg = rgoproto::RealtimePoseStreamingMsg;
        using AsyncService = rgoproto::LiveSlamUIService::AsyncService;
        using StreamMethodT = decltype(&rgoproto::LiveSlamUIService::AsyncService::RequestRealtimePoseStreaming);

        static constexpr StreamMethodT StreamRequestMethod = &rgoproto::LiveSlamUIService::AsyncService::RequestRealtimePoseStreaming;

        static constexpr char PRETTY_NAME[] = "RT Pose Stream";


    };

    template<typename StreamPolicy>
    class GenericServerToClientStreamer : public std::enable_shared_from_this<GenericServerToClientStreamer<StreamPolicy>>
    {
        using StreamStartMsgT = typename StreamPolicy::StreamRequestMsg;
        using StreamMsgT = typename StreamPolicy::StreamMsg;
        using AsyncServiceT = typename StreamPolicy::AsyncService;

        std::shared_ptr<GrpcContext>                                 _grpcContext;
        std::shared_ptr<AsyncServiceT>                               _service;

        //
        // state
        //
        std::atomic_bool                                             _shutdown{ false };

        // session
        SafeAlarm                                                    _startAlarm;
        SafeAlarm                                                    _stopAlarm;
        SafeAlarm                                                    _checkDataAlarm;
        HandlerState                                                 _state = HandlerState::Inactive;
        std::shared_ptr<grpc::ServerContext>                         _serverContext;
        std::unique_ptr<grpc::ServerAsyncWriter<StreamMsgT>>         _asyncWriter;
        std::size_t                                                  _packetsCount{ 0 };

        StreamStartMsgT                                              _clientRequest;
        std::shared_ptr<StreamMsgT>                                  _pendingStreamMsg;
        std::shared_ptr<StreamMsgT>                                  _writingStreamMsg;
        std::shared_ptr<StreamMsgT>                                  _writtenStreamMsg;


        struct PrivatePassKey {}; // only allow creation via Make that calls make_shared
    public:
        static std::shared_ptr<GenericServerToClientStreamer> Make(std::shared_ptr<AsyncServiceT> service, std::shared_ptr<GrpcContext> grpcContext)
        {
            return std::make_shared<GenericServerToClientStreamer>(std::move(service), std::move(grpcContext), PrivatePassKey{});
        }

        GenericServerToClientStreamer(
            std::shared_ptr<AsyncServiceT> service,
            std::shared_ptr<GrpcContext> grpcContext,
            PrivatePassKey
        )
            :
            _grpcContext(grpcContext),
            _service(std::move(service)),
            _startAlarm(_grpcContext),
            _stopAlarm(_grpcContext),
            _checkDataAlarm(_grpcContext)
        {

        }

        ~GenericServerToClientStreamer()
        {
            DEBUGGER_TRACE("streaming dtor");

            assert(_shutdown);  // did you call Shutdown?
            assert(_state == HandlerState::Inactive);
        }

        HandlerState State() const { return _state; };

        void Start()
        {
            _startAlarm.Trigger(_grpcContext->Get(),
                [self = this->weak_from_this()](bool ok)
                {
                    if (auto s = self.lock())
                    {
                        s->HandleStartListening(ok);
                    }
                });
        }

        void Stop()
        {
            _stopAlarm.Trigger(_grpcContext->Get(),
                [self = this->weak_from_this()](bool ok)
                {
                    if (auto s = self.lock())
                    {
                        s->HandleStopStreaming(ok);
                    }
                });
        }

        void Shutdown()
        {
            assert(_shutdown == false);
            _shutdown.store(true);

            Stop();

            while (_state != HandlerState::Inactive)
            {
                _grpcContext->RunFor(1ms);
            }
        }

        std::shared_ptr<StreamMsgT> ExchangeData(std::shared_ptr<StreamMsgT> msg)
        {
            auto writtenData = std::move(_writtenStreamMsg);
            _pendingStreamMsg = std::move(msg);
            _checkDataAlarm.Trigger(_grpcContext->Get(),
                [self = this->weak_from_this()](bool ok)
                {
                    if (auto s = self.lock())
                    {
                        s->HandleCheckForData(ok);
                    }
                });
            return writtenData;
        }

    private:
        bool IsStreaming() const
        {
            return _state == HandlerState::WaitingForAvailableData ||
                _state == HandlerState::WaitngForWriteDone;
        }

        void HandleStartListening(bool ok)
        {
            _startAlarm.Reset();
            if (_state == HandlerState::Inactive)
            {
                _serverContext = std::make_shared<grpc::ServerContext>();

                auto weakServerContext = std::weak_ptr<grpc::ServerContext>(_serverContext);

                auto cancelTag = _grpcContext->CreateTag(
                    [weakServerContext, self = this->weak_from_this()](bool ok) mutable
                    {
                        if (auto s = self.lock())
                        {
                            s->HandleStreamCancel(ok, std::move(weakServerContext));
                        }
                    }
                );

                _serverContext->AsyncNotifyWhenDone(cancelTag);

                _asyncWriter = std::make_unique<grpc::ServerAsyncWriter<StreamMsgT>>(_serverContext.get());

                auto tag = _grpcContext->CreateTag(
                    [self = this->weak_from_this()](bool ok) mutable
                    {
                        if (auto s = self.lock())
                        {
                            s->HandleStartStreaming(ok);
                        }
                    }
                );

                constexpr auto method = StreamPolicy::StreamRequestMethod;
                auto self = _service.get();

                (*self.*method) // magic
                    (
                        _serverContext.get(),
                        &_clientRequest,
                        _asyncWriter.get(),
                        _grpcContext->Get(),
                        _grpcContext->Get(),
                        tag
                        );

                _state = HandlerState::Listening;
                DEBUGGER_TRACE("Service - listening waiting for streaming to start");
            }

            DEBUGGER_TRACE("Service - HandleStartListening {} ", ok);
        }

        void HandleStartStreaming(bool ok)
        {
            if (ok && _state == HandlerState::Listening)
            {
                _state = HandlerState::WaitingForAvailableData;

                PollPendingDataAndWrite();
            }
            else
            {
                // probably a shutdown
                HandleStreamError();
            }

            DEBUGGER_TRACE("Service - HandleStartStreaming {} ", ok);
        }

        void HandleStopStreaming(bool ok)
        {
            _stopAlarm.Reset();
            if (ok)
            {
                if (IsStreaming())
                {
                    auto tag = _grpcContext->CreateTag(
                        [self = this->weak_from_this()](bool ok) mutable
                        {
                            if (auto s = self.lock())
                            {
                                s->HandleStreamFinish(ok);
                            }
                        }
                    );

                    // finish gracefully
                    _state = HandlerState::Stopping;
                    grpc::Status status(grpc::OK, "stopped");
                    _asyncWriter->Finish(status, tag);
                    DEBUGGER_TRACE("server - received stop, finishing");
                }
                else if (_state == HandlerState::Listening)
                {
                    // nothing to do, only a shutdown can cancel a streaming request
                }
            }
            else
            {
                HandleStreamError();
            }

            DEBUGGER_TRACE("server - HandleStopStreaming {} ", ok);
        }

        void PollPendingDataAndWrite()
        {
            if (_state == HandlerState::WaitingForAvailableData)
            {
                auto data = std::move(_pendingStreamMsg);

                if (data)
                {
                    auto tag = _grpcContext->CreateTag(
                        [self = this->weak_from_this()](bool ok) mutable
                        {
                            if (auto s = self.lock())
                            {
                                s->HandleWriteDone(ok);
                            }
                        }
                    );

                    _writingStreamMsg = std::move(data);
                    _asyncWriter->Write(*_writingStreamMsg, tag);
                    _state = HandlerState::WaitngForWriteDone;
                }
            }
        }

        void HandleWriteDone(bool ok)
        {
            if (ok && _state == HandlerState::WaitngForWriteDone)
            {
                ++_packetsCount;
                _writtenStreamMsg = std::move(_writingStreamMsg);
                _state = HandlerState::WaitingForAvailableData;
                PollPendingDataAndWrite();
            }
            else if (_state == HandlerState::Cancelled || _state == HandlerState::Stopping)
            {
                DEBUGGER_TRACE("Service - HandleWriteDone cancel or stopped");
            }
            else
            {
                HandleStreamError();
            }
        }

        void HandleStreamCancel(bool ok, std::weak_ptr<grpc::ServerContext> serverContextWeak)
        {
            assert(ok); // Server-side AsyncNotifyWhenDone: ok should always be true

            // in case finish handler was called before
            auto serverContext = serverContextWeak.lock();

            if (serverContext && serverContext->IsCancelled())
            {
                // cancelled by the client
                _state = HandlerState::Cancelled;
                grpc::Status status(grpc::CANCELLED, "cancelled");

                auto tag = _grpcContext->CreateTag(
                    [self = this->weak_from_this()](bool ok)
                    {
                        if (auto s = self.lock())
                        {
                            s->HandleStreamFinish(ok);
                        }
                    }
                );
                _asyncWriter->Finish(status, tag);

                DEBUGGER_TRACE("Service - received cancel, finishing");
            }
            else
            {
                DEBUGGER_TRACE("Service - received cancel but IsCancelled is FALSE or stream already finished");
            }

            DEBUGGER_TRACE("Service - received cancel, HandleStreamCancel {} ", ok);
        }

        void HandleStreamFinish(bool ok)
        {
            DEBUGGER_TRACE("Service - received stream, HandleStreamFinish {} ", ok);

            if (_state == HandlerState::Cancelled)
            {
                // client cancelled, start listening again
                _state = HandlerState::Inactive;

                // client cancelled and no streaming continues
                // we need to schedule start streaming

                if (!_shutdown)
                {
                    DEBUGGER_TRACE("service - HandleStreamFinish {} - restart listening", ok);
                    Start();
                }

            }
            else if (ok && _state == HandlerState::Stopping)
            {
                // gracefully stop
                _state = HandlerState::Inactive;
            }
            else
            {
                HandleStreamError();
            }


            _serverContext.reset();
            _asyncWriter.reset();
        }

        void HandleCheckForData(bool ok)
        {
            _checkDataAlarm.Reset();
            if (ok)
            {
                PollPendingDataAndWrite();
            }
        }

        void HandleStreamError()
        {
            _state = HandlerState::Inactive;
            DEBUGGER_TRACE("server - HandleStreamError");
        }
    };
}

