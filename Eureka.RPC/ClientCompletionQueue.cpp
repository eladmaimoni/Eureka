#include "ClientCompletionQueue.hpp"
#include <thread_name.hpp>
#include <logging.hpp>

namespace eureka
{


    ClientCompletionQueueExecutor::ClientCompletionQueueExecutor(std::unique_ptr<grpc::CompletionQueue> completionQueue)
        :
        _grpcContext(std::move(completionQueue)),
        _workGuard(asio::make_work_guard(_grpcContext))
    {
        _thread = std::jthread(
            [this]
            {
                try
                {
                    eureka::set_current_thread_name("eureka grpc client thread");
                    _grpcContext.run();
                }
                catch (const std::exception& err)
                {
                    CLOG("{}", err.what());
                }
            });
    }

    ClientCompletionQueueExecutor::~ClientCompletionQueueExecutor()
    {
        _workGuard.reset();
    }

}

