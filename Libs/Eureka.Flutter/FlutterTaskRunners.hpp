#pragma once
//#include <boost/container/flat_set.hpp>
#include <set>
#include <flutter/flutter_embedder.h>
#include <mutex>
#include <thread>

namespace eureka::flutter
{

    struct PendingTask
    {
        FlutterTask task;
        uint64_t    target_time_nanos;
    };

    inline bool operator<(const PendingTask& lhs, const PendingTask& rhs)
    {
        return lhs.target_time_nanos < rhs.target_time_nanos;
    }

    class TaskRunner
    {
    protected:
        std::mutex                              _mtx;
        std::condition_variable                 _cv;
        FlutterEngine                           _engine;
        FlutterTaskRunnerDescription            _description;
        std::thread::id                         _executionId;
        std::set<PendingTask>                   _pending;

        static bool IsTaskRunOnCurrentThreadStatic(void* userData)
        {
            auto self = static_cast<TaskRunner*>(userData);
            return self->IsTaskRunOnCurrentThread();
        }

        static void PostTaskStatic(FlutterTask task, uint64_t targetTimeNanos, void* userData)
        {
            auto self = static_cast<TaskRunner*>(userData);
            self->PostTask(task, targetTimeNanos);
        }


        bool IsTaskRunOnCurrentThread()
        {
            return (std::this_thread::get_id() == _executionId);
        }


        void PostTask(FlutterTask task, uint64_t targetTimeNanos);

    public:
        TaskRunner(std::thread::id executionId);

        void SetEngineHandle(FlutterEngine engine);
        const FlutterTaskRunnerDescription& GetDescription() const;

        void RunReadyTasksFor(std::chrono::milliseconds duration);
    };

} // namespace eureka::flutter