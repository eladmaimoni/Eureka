#pragma once
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4127 4702)
#if defined(_WIN64) || defined(WIN64) || defined(_WIN32) || defined(WIN32)
#include <SDKDDKVer.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif
#include <agrpc/asio_grpc.hpp>
EUREKA_MSVC_WARNING_POP
#include <thread>

namespace eureka
{
    class ServerCompletionQueueExecutor
    {
        using GrpcExecutor = agrpc::GrpcContext::executor_type;

        agrpc::GrpcContext                        _grpcContext;
        asio::executor_work_guard<GrpcExecutor>   _workGuard;
        std::jthread                              _thread;
    public:
        ServerCompletionQueueExecutor(std::unique_ptr<grpc::ServerCompletionQueue> completionQueue);

        ~ServerCompletionQueueExecutor();

        agrpc::GrpcContext& Get()
        {
            return _grpcContext;
        }
    };


}

