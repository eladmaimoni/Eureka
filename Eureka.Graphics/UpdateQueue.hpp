
namespace eureka
{
    inline thread_local bool tls_is_rendering_thread = false;

    class RenderingUpdateQueue
    {
    public:
        void UpdatePreRender();
        void Update();
        void EnqueuePreRenderUpdate(fu::unique_function<void(void)> f);
        void EnqueueUpdate(fu::unique_function<void(void)> f);
    private:

        std::mutex                                    _mtx;
        bool                                          _updating{ false };
        std::deque<fu::unique_function<void(void)>>   _preRenderFuncs;
        std::deque<fu::unique_function<void(void)>>   _funcs;

        void EnqueueUpdate(fu::unique_function<void(void)> f, std::deque<fu::unique_function<void(void)>>& deque);
        void DoUpdate(std::deque<fu::unique_function<void(void)>>& deque);
    };



}