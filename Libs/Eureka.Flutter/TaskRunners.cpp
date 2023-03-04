#include "TaskRunners.hpp"
#include "FlutterUtils.hpp"
#include <debugger_trace.hpp>
#include <profiling.hpp>

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
    
    void TaskRunner::RunReadyTasksFor(std::chrono::milliseconds duration)
    {
        PROFILE_CATEGORIZED_SCOPE("RunReadyTasksFor", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_SYSTEM);
        auto now = std::chrono::high_resolution_clock::now();
        auto until = now + duration;

        while (now < until)
        {
            std::unique_lock lk(_mtx);
            auto currentTime = FlutterEngineGetCurrentTime();
            bool haveReadyTasks = !_pending.empty() && _pending.begin()->target_time_nanos <= currentTime;

            //auto status = std::cv_status::no_timeout;

            if (!haveReadyTasks /*&& status != std::cv_status::timeout*/)
            {
               /* status =*/ _cv.wait_until(lk, until);
            }

            auto itr = _pending.begin();
            auto eitr = _pending.end();

            while (itr != eitr)
            {
                currentTime = FlutterEngineGetCurrentTime();
                auto [task, targetTime] = *itr;

                if (targetTime <= currentTime)
                {
                    _pending.erase(itr++);
                    lk.unlock();

                    PROFILE_CATEGORIZED_SCOPE("FlutterEngineRunTask", profiling::Color::Green, profiling::PROFILING_CATEGORY_SYSTEM);
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

            now = std::chrono::high_resolution_clock::now();
        }
 
    }

    void TaskRunner::PostTask(FlutterTask task, uint64_t targetTimeNanos)
    {
        std::scoped_lock lk(_mtx);
        _pending.emplace(PendingTask{ task, targetTimeNanos });
        _cv.notify_one();
    }
}