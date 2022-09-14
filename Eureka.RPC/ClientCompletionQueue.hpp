#pragma once
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <agrpc/asio_grpc.hpp>
EUREKA_MSVC_WARNING_POP
#include <thread>

namespace eureka
{
    class ClientCompletionQueueExecutor
    {
        using GrpcExecutor = agrpc::GrpcContext::executor_type;

        agrpc::GrpcContext                        _grpcContext;
        asio::executor_work_guard<GrpcExecutor>   _workGuard;
        std::jthread                              _thread;
    public:
        ClientCompletionQueueExecutor(std::unique_ptr<grpc::CompletionQueue> completionQueue);

        ~ClientCompletionQueueExecutor();

        agrpc::GrpcContext& Get()
        {
            return _grpcContext;
        }
    };
}

