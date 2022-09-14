#include "ServerCompletionQueue.hpp"
#include <thread_name.hpp>
#include <logging.hpp>

namespace eureka
{


    ServerCompletionQueueExecutor::ServerCompletionQueueExecutor(
        std::unique_ptr<grpc::ServerCompletionQueue> completionQueue
    ) :
        _grpcContext(std::move(completionQueue)),
        _workGuard(asio::make_work_guard(_grpcContext))
    {
        _thread = std::jthread(
            [this]
            {
                try
                {
                    eureka::set_current_thread_name("eureka grpc server thread");
                    _grpcContext.run();
                }
                catch (const std::exception& err)
                {
                    CLOG("{}", err.what());
                }
            });
    }

    ServerCompletionQueueExecutor::~ServerCompletionQueueExecutor()
    {
        _workGuard.reset();
        //_pool.join();
    }



}

