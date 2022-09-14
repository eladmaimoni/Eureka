#include "UnifiedCompletionQueue.hpp"
#include <logging.hpp>
#include <thread_name.hpp>

namespace eureka
{
    UnifiedCompletionQueuesExecutor::UnifiedCompletionQueuesExecutor(
        std::unique_ptr<grpc::ServerCompletionQueue> completionQueue
    ) :
        _grpcContext(std::move(completionQueue)),
        _workGuard(asio::make_work_guard(_grpcContext)),
        _pool(1)
    {
        asio::post(_ioContext,
            [this]
            {
                try
                {
                    _ioContext.get_executor().on_work_finished();
                    agrpc::run(_grpcContext, _ioContext);
                    _ioContext.get_executor().on_work_started();
                }
                catch (const std::exception& err)
                {
                    CLOG("{}", err.what());
                }

            });

        asio::post(_pool, [this]
            {
                try
                {
                    eureka::set_current_thread_name("zzz pool thread");
                    _ioContext.run();
                }
                catch (const std::exception& err)
                {
                    CLOG("{}", err.what());
                }

            }
        );
    }

    UnifiedCompletionQueuesExecutor::~UnifiedCompletionQueuesExecutor()
    {
        _workGuard.reset();
    }

}

