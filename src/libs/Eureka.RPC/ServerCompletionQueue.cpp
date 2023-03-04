#include "ServerCompletionQueue.hpp"
#include <thread_name.hpp>
#include <logging.hpp>
using namespace std::chrono_literals;

namespace eureka
{


    ServerCompletionQueueExecutor::ServerCompletionQueueExecutor(
        std::unique_ptr<grpc::ServerCompletionQueue> completionQueue
    ) :
        _grpcContext(std::move(completionQueue)),
        _workGuard(asio::make_work_guard(_grpcContext))
    {
        _thread = jthread(
            [this]
            {
                try
                {
                    eureka::os::set_current_thread_name("eureka grpc server thread");

                    while (true)
                    {
                        _grpcContext.run_until(std::chrono::system_clock::now() + 1s);
                    }
             
                }
                catch (const std::exception& err)
                {
                    SPDLOG_ERROR("{}", err.what());
                }
            });
    }

    ServerCompletionQueueExecutor::~ServerCompletionQueueExecutor()
    {
        _workGuard.reset();
        //_pool.join();
    }



}

