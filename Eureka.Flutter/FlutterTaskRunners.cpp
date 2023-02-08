#include "FlutterTaskRunners.hpp"
#include "FlutterUtils.hpp"
#include <debugger_trace.hpp>

namespace eureka::flutter
{
    TaskRunner::TaskRunner(std::thread::id executionId) :
        _executionId(executionId)
    {
        _description.struct_size = sizeof(FlutterTaskRunnerDescription);
        _description.user_data = this;
        _description.runs_task_on_current_thread_callback = IsTaskRunOnCurrentThreadStatic;
        _description.post_task_callback = PostTaskStatic;
    }

    void TaskRunner::SetEngineHandle(FlutterEngine engine)
    {
        _engine = engine;
    }

    const FlutterTaskRunnerDescription& TaskRunner::GetDescription() const
    {
        return _description;
    }
    
    void TaskRunner::PollReadyTasks()
    {
        std::unique_lock lk(_mtx);

        auto itr = _pending.begin();
        auto eitr = _pending.end();

        while (itr != eitr)
        {
            auto currentTime = FlutterEngineGetCurrentTime();
            auto [task, targetTime] = *itr;

            if (targetTime <= currentTime)
            {
                _pending.erase(itr++);
                lk.unlock();
                //DEBUGGER_TRACE("run task start");
                FLUTTER_CHECK(FlutterEngineRunTask(_engine, &task));
                //DEBUGGER_TRACE("run task end");
                lk.lock();
            }
            else
            {
                // tasks are sorted by target time
                break;
            }
        }
    }

    void TaskRunner::PostTask(FlutterTask task, uint64_t targetTimeNanos)
    {
        std::scoped_lock lk(_mtx);
        _pending.emplace(PendingTask{ task, targetTimeNanos });
    }
}