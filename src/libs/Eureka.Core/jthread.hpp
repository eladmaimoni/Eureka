#pragma once

#include <thread>
#include <string>



namespace eureka
{
#ifdef __cpp_lib_jthread
	using jthread = std::jthread;
#else
	class jthread
	{
	public:
		jthread() = default;
		jthread(const jthread& that) = delete;
		jthread(jthread&& that) = default;
		jthread& operator=(const jthread& rhs) = delete;
        jthread& operator=(jthread&& rhs)
        {
            join_if_joinable();
            _thread = std::move(rhs._thread);
            return *this;
            
        };


		template <class Function, class... Args>
		explicit jthread(Function&& f, Args&&... args)
			: _thread(std::forward<Function>(f), std::forward<Args>(args)...)
		{

		}

		void detach() { _thread.detach(); }
		void join() { _thread.join(); }
		std::thread& get() { return _thread; }

		~jthread()
		{
            join_if_joinable();
		}
	private:
		std::thread _thread;
        void join_if_joinable()
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }
	};
#endif
}

