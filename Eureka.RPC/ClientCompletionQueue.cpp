#include "ClientCompletionQueue.hpp"
#include <thread_name.hpp>
#include <logging.hpp>
#include <profiling.hpp>

namespace eureka
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ClientCompletionQueueExecutor::ClientCompletionQueueExecutor()
        :
        //_grpcContext(),
        _workGuard(asio::make_work_guard(_grpcContext))
    {

    }

    ClientCompletionQueueExecutor::~ClientCompletionQueueExecutor()
    {
        _workGuard.reset();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ClientCompletionQueueThreadExecutor::ClientCompletionQueueThreadExecutor()
    {
        _thread = std::jthread(
            [this]
            {
                try
                {
                    eureka::os::set_current_thread_name("eureka grpc client thread");
                    _grpcContext.run();
                }
                catch (const std::exception& err)
                {
                    CLOG("{}", err.what());
                }
            });
    }


    ClientCompletionQueueThreadExecutor::~ClientCompletionQueueThreadExecutor()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    ClientCompletionQueuePollingExecutor::ClientCompletionQueuePollingExecutor()
    {

    }

    ClientCompletionQueuePollingExecutor::~ClientCompletionQueuePollingExecutor()
    {

    }

    void ClientCompletionQueuePollingExecutor::RunCompletions()
    {
        _grpcContext.run();
    }

    void ClientCompletionQueuePollingExecutor::PollCompletions()
    {
        _grpcContext.poll();
    }

    void ClientCompletionQueuePollingExecutor::PollCompletions(std::chrono::nanoseconds duration)
    {
        PROFILE_CATEGORIZED_SCOPE("run_until", eureka::profiling::Color::Red, eureka::profiling::PROFILING_CATEGORY_SYSTEM);
        auto grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        auto grpc_duration = gpr_time_from_nanos(duration.count(), gpr_clock_type::GPR_TIMESPAN);
        auto grpc_until = gpr_time_add(grpc_now, grpc_duration);

        _grpcContext.run_until(grpc_until);
     
    }
}

