#include "UpdateQueue.hpp"

namespace eureka
{

    void RenderingThreadUpdateQueue::UpdatePreRender()
    {
         DoUpdate(_preRenderFuncs);
         DoUpdate(_funcs);
    }

    void RenderingThreadUpdateQueue::Update()
    {

        DoUpdate(_funcs);
    }

    void RenderingThreadUpdateQueue::EnqueuePreRenderUpdate(fu::unique_function<void(void)> f)
    {
        EnqueueUpdate(std::move(f), _preRenderFuncs);
    }
    void RenderingThreadUpdateQueue::EnqueueUpdate(fu::unique_function<void(void)> f)
    {
        EnqueueUpdate(std::move(f), _funcs);
    }

    void RenderingThreadUpdateQueue::EnqueueUpdate(fu::unique_function<void(void)> f, std::deque<fu::unique_function<void(void)>>& deque)
    {
        std::unique_lock lk(_mtx);
        if (_updating && tls_is_rendering_thread)
        {
            // a call from within the rendering thread: invoke immediately
            lk.unlock();
            f();
        }
        else
        {
            deque.emplace_back(std::move(f));
        }
    }



    void RenderingThreadUpdateQueue::DoUpdate(std::deque<fu2::unique_function<void(void)>>& deque)
    {
        if (!deque.empty())
        {
            std::unique_lock lk(_mtx);
            _updating = true;

            fu::unique_function<void(void)> updateFunc;

            while (!deque.empty())
            {
                updateFunc = std::move(deque.front());
                deque.pop_front();
                lk.unlock();
                updateFunc();
                lk.lock();
            }
            _updating = false;
        }
    }

}