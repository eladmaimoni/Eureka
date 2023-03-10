#pragma once
#include "LiveSlamServiceHelpers.hpp"
#include "GrpcContext.hpp"

EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702)
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP

namespace eureka::rpc
{
    enum class UnaryRPCState
    {
        Inactive,
        Listening,
        RequestFulfilled
    };

    struct ForceFullGPORPCPolicy
    {
        using RequestMsg = rgoproto::ForceFullGPORequestMsg;
        using ResponseMsg = rgoproto::ForceFullGPOResponseMsg;
        using AsyncService = rgoproto::LiveSlamUIService::AsyncService;
        using RPCMethodT = decltype(&rgoproto::LiveSlamUIService::AsyncService::RequestForceFullGPO);
        static constexpr RPCMethodT RPCRequestMethod = &rgoproto::LiveSlamUIService::AsyncService::RequestForceFullGPO;
    };

    template<typename RPCPolicy>
    class GenericImmediateUnaryRPC : public std::enable_shared_from_this<GenericImmediateUnaryRPC<RPCPolicy>>
    {
        //
        // GenericImmediateUnaryRPC
        // RPC handler for a unary request arriving from the client
        // generally, the server must fulfill the request by writing a proper response
        // the user may write the response immediately upon arriving request or asynchronously.
        // this RPC handler assumes the user would like to complete the RPC immediately
        // otherwise, we must supply our user with a callback that will Finish the RPC
        using RequestMsgT = typename RPCPolicy::RequestMsg;
        using ResponseMsgT = typename RPCPolicy::ResponseMsg;
        using AsyncServiceT = typename RPCPolicy::AsyncService;
        using UserResponseWriter = std::function<void(const RequestMsgT&, ResponseMsgT&)>;

        std::shared_ptr<GrpcContext>                                           _grpcContext;
        std::shared_ptr<AsyncServiceT>                                         _service;

        //                                                                     
        // state                                                               
        //                                                                     
        std::atomic_bool                                                       _shutdown{ false };

        // session                                                             
        SafeAlarm                                                              _startAlarm;
        SafeAlarm                                                              _stopAlarm;
        UnaryRPCState                                                          _state = UnaryRPCState::Inactive;
        std::shared_ptr<grpc::ServerContext>                                   _serverContext;
        std::shared_ptr<grpc::ServerAsyncResponseWriter<ResponseMsgT>>         _responder;
        UserResponseWriter                                                     _userResponseWriter;
        std::size_t                                                            _packetsCount{ 0 };

        struct PrivatePassKey {}; // only allow creation via Make that calls make_shared
    public:
        static std::shared_ptr<GenericImmediateUnaryRPC> Make(std::shared_ptr<AsyncServiceT> service, std::shared_ptr<GrpcContext> grpcContext)
        {
            return std::make_shared<GenericImmediateUnaryRPC>(std::move(service), std::move(grpcContext), PrivatePassKey{});
        }

        GenericImmediateUnaryRPC(
            std::shared_ptr<AsyncServiceT> service,
            std::shared_ptr<GrpcContext> grpcContext,
            PrivatePassKey
        )
            :
            _grpcContext(grpcContext),
            _service(std::move(service)),
            _startAlarm(_grpcContext),
            _stopAlarm(_grpcContext)
        {

        }

        ~GenericImmediateUnaryRPC()
        {
            DEBUGGER_TRACE("streaming dtor");

            assert(_shutdown);  // did you call Shutdown?
            assert(_state == UnaryRPCState::Inactive);
        }

        UnaryRPCState State() const { return _state; };

        void Start(UserResponseWriter userResponseWriter)
        {
            _startAlarm.Trigger(_grpcContext->Get(),
                [self = this->weak_from_this(), userResponseWriter = std::move(userResponseWriter)](bool ok) mutable
                {
                    
                    if (auto s = self.lock())
                    {
                        s->_userResponseWriter = std::move(userResponseWriter);
                        s->HandleStartListening(ok);
                    }
                });
        }

        void Stop()
        {
            _stopAlarm.Trigger(_grpcContext->Get(),
                [self = this->weak_from_this()](bool /*ok*/) mutable
                {
                    if (auto s = self.lock())
                    {
                        s->_userResponseWriter = [](const RequestMsgT&, ResponseMsgT&) {};
                    }
                });
        }
    private:
        void HandleStartListening(bool ok)
        {
            _startAlarm.Reset();
            if (_state == UnaryRPCState::Inactive)
            {
                DoStartListening();

                DEBUGGER_TRACE("Service - listening waiting for streaming to start");
            }

            DEBUGGER_TRACE("Service - HandleStartListening {} ", ok);
        }


        void HandleRequest(bool ok, std::shared_ptr<RequestMsgT> request)
        {
            if (ok && _state == UnaryRPCState::Listening)
            {

                // call user callback with a given callback
                auto response = std::make_shared<ResponseMsgT>();

                _userResponseWriter(*request, *response);

                _state = UnaryRPCState::RequestFulfilled;

                auto tag = _grpcContext->CreateTag(
                    [response, self = this->weak_from_this()](bool ok) mutable
                    {
                        if (auto s = self.lock())
                        {
                            s->HandleFinish(ok);
                        }
                    }
                );
                grpc::Status status;
                _responder->Finish(*response, status, tag);
            }
            else
            {
                // probably a shutdown
                HandleRPCError();
            }

            DEBUGGER_TRACE("Service - HandleStartStreaming {} ", ok);
        }

        void HandleFinish(bool ok)
        {
            _serverContext.reset();
            _responder.reset();
            DEBUGGER_TRACE("Service - HandleFinish {} ", ok);

            if (ok && _state == UnaryRPCState::RequestFulfilled)
            {
                // listen to next request
                DoStartListening();
            }
            else
            {
                HandleRPCError();
            }

        }

        void HandleRPCError()
        {
            _state = UnaryRPCState::Inactive;
            DEBUGGER_TRACE("server - HandleRPCError");
        }

        void DoStartListening()
        {
            _serverContext = std::make_shared<grpc::ServerContext>();
            _responder = std::make_shared<grpc::ServerAsyncResponseWriter<ResponseMsgT>>(_serverContext.get());
            auto requestMsg = std::make_shared<RequestMsgT>();

            auto tag = _grpcContext->CreateTag(
                [requestMsg, self = this->weak_from_this()](bool ok) mutable
                {
                    if (auto s = self.lock())
                    {
                        s->HandleRequest(ok, std::move(requestMsg));
                    }
                }
            );

            constexpr auto method = RPCPolicy::RPCRequestMethod;
            auto self = _service.get();

            (*self.*method) // magic
                (
                    _serverContext.get(),
                    requestMsg.get(),
                    _responder.get(),
                    _grpcContext->Get(),
                    _grpcContext->Get(),
                    tag
                    );

            _state = UnaryRPCState::Listening;
        }



    };
}

