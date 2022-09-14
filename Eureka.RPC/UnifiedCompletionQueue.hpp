#pragma once
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#include <agrpc/asio_grpc.hpp>
#include <asio/io_context.hpp>
#include <asio/thread_pool.hpp>
EUREKA_MSVC_WARNING_POP

namespace eureka
{

    class UnifiedCompletionQueuesExecutor
    {
        using GrpcExecutor = agrpc::GrpcContext::executor_type;

        asio::io_context                          _ioContext;
        agrpc::GrpcContext                        _grpcContext;
        asio::executor_work_guard<GrpcExecutor>   _workGuard;
        asio::thread_pool                         _pool;
    public:
        UnifiedCompletionQueuesExecutor(
            std::unique_ptr<grpc::ServerCompletionQueue> completionQueue
        );

        ~UnifiedCompletionQueuesExecutor();

        agrpc::GrpcContext& Get()
        {
            return _grpcContext;
        }

    };



}

