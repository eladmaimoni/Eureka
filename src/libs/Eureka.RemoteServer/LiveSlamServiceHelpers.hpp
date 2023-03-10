#pragma once
#include <compiler.hpp>

EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702 4127)
#include <grpcpp/server_builder.h>
#include <grpcpp/alarm.h>
EUREKA_MSVC_WARNING_POP
#include "GrpcContext.hpp"
namespace eureka::rpc
{
    inline auto GrpcTimpointFromNow(std::chrono::nanoseconds duration)
    {
        auto grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        auto grpc_duration = gpr_time_from_nanos(duration.count(), gpr_clock_type::GPR_TIMESPAN);
        auto grpc_timpoint = gpr_time_add(grpc_now, grpc_duration);

        return grpc_timpoint;
    }



    //
    // grpc::Alarm can't be re-'Set' before the pevious 'Set' completion tag is out of the completion queue
    // so this wrapper makes sure we won't be able to trigger an alarm before we explicitly Reset it 
    //
    class SafeAlarm
    {
    private:
        std::shared_ptr<GrpcContext> _grpcContext;
        grpc::Alarm _alarm;
        std::atomic_bool _tiggered{ false };
    public:
        SafeAlarm(std::shared_ptr<GrpcContext> grpcContext)
            : _grpcContext(std::move(grpcContext))
        {

        }
        void Trigger(grpc::CompletionQueue* cq, CompletionHandler completionHandler, std::optional<uint64_t> strandId = std::nullopt)
        {
            bool expected = false;
            if (_tiggered.compare_exchange_weak(expected, true))
            {
                auto tag = _grpcContext->CreateTag(std::move(completionHandler), strandId);
                _alarm.Set(cq, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), tag);
            }
        }
        void Reset()
        {
            _tiggered.store(false, std::memory_order_relaxed);
        }
    };

}

