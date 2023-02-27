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
#include <jthread.hpp>

namespace asio
{
    template<typename T>
    using awaitable_optional = asio::awaitable<std::optional<T>>;
}

namespace eureka
{
    inline std::string to_string(grpc_connectivity_state connectionState)
    {
        std::string result = "unknown state";
        switch (connectionState)
        {
        case grpc_connectivity_state::GRPC_CHANNEL_CONNECTING:
            result = "connecting";
            break;
        case grpc_connectivity_state::GRPC_CHANNEL_IDLE:
            result = "idle";
            break;
        case grpc_connectivity_state::GRPC_CHANNEL_READY:
            result = "ready";
            break;
        case grpc_connectivity_state::GRPC_CHANNEL_TRANSIENT_FAILURE:
            result = "failure";
            break;
        case grpc_connectivity_state::GRPC_CHANNEL_SHUTDOWN:
            result = "shutdown";
            break;
        }
        return result;
    }

    inline auto grpc_deadline_from_now(std::chrono::nanoseconds duration)
    {
        auto grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        auto grpc_duration = gpr_time_from_nanos(duration.count(), gpr_clock_type::GPR_TIMESPAN);
        auto grpc_deadline = gpr_time_add(grpc_now, grpc_duration);
        return grpc_deadline;
    }

    class ClientCompletionQueueExecutor
    {
    protected:
        using GrpcExecutor = agrpc::GrpcContext::executor_type;

        agrpc::GrpcContext                        _grpcContext;
        asio::executor_work_guard<GrpcExecutor>   _workGuard;       
    public:
        ClientCompletionQueueExecutor();

        virtual ~ClientCompletionQueueExecutor();

        agrpc::GrpcContext& Get()
        {
            return _grpcContext;
        }

        decltype(auto) GetCompletionToken()
        {
            return asio::bind_executor(_grpcContext, asio::use_awaitable);
        }

    };

    class ClientCompletionQueueThreadExecutor : public ClientCompletionQueueExecutor
    {
        jthread  _thread;
    public:
        ClientCompletionQueueThreadExecutor();
        ~ClientCompletionQueueThreadExecutor();
    };

    class ClientCompletionQueuePollingExecutor : public ClientCompletionQueueExecutor
    {
    public:
        ClientCompletionQueuePollingExecutor();
        ~ClientCompletionQueuePollingExecutor();
        void RunCompletions();
        void PollCompletions();
        void PollCompletions(std::chrono::nanoseconds duration);

    };
}

