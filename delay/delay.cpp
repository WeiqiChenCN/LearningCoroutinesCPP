// delay.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <future>
#include <thread>
#include <utility>
#include <iostream>
#include <coroutine>

#include <Windows.h>


#if false
static VOID CALLBACK TimerCallback(
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_TIMER             Timer
)
{
	std::cout << "Timer expires, in thread ("
		<< std::this_thread::get_id() << ")." << std::endl;
	CloseThreadpoolTimer(Timer);
}

template< class Rep, class Period >
void start_a_timer(const std::chrono::duration<Rep, Period>& sleep_duration)
{
	auto timer = CreateThreadpoolTimer(
		TimerCallback,
		nullptr, nullptr);
	if (timer == nullptr)
	{
		throw new std::runtime_error("timer created failure.");
	}
	auto count = std::chrono::nanoseconds(sleep_duration).count() / 100 * -1;
	FILETIME ft;
	memcpy(&ft, &count, sizeof(INT64));
	SetThreadpoolTimer(timer, &ft, 0, 0);
}


int main()
{
	using namespace std::chrono_literals;
	start_a_timer(1s);
	while (true) {};

}

#endif

struct delay_task
{
	struct promise_type;
	using handle = std::coroutine_handle<promise_type>;
	handle m_handle;

	delay_task(std::coroutine_handle<promise_type> h) :
		m_handle(h)
	{
		//std::cout << "delay_task with handled(" << m_handle.address() << ") was created." << std::endl;
	}
	delay_task(delay_task&& other) noexcept
	{
		m_handle = std::move(other.m_handle);
		other.m_handle = {};
	}
	~delay_task()
	{
		//std::cout << "delay_task with handled(" << m_handle.address() << ") was deleted." << std::endl;
		if(m_handle)
			m_handle.destroy();
	}

	struct promise_type
	{
		struct final_awaiter;
		//T result;
		std::coroutine_handle<> previous;
		std::chrono::milliseconds duration;
		promise_type(std::chrono::milliseconds duration)
		{
			this->duration = duration;
		}
		~promise_type()
		{
			std::cout << "~delay_task::promise_type in thread id(" << std::this_thread::get_id() << ")" << std::endl;
		}
		delay_task get_return_object()
		{
			/*pass promise obj to task::m_handle*/
			return delay_task {handle::from_promise(*this)};
		}
		std::suspend_always initial_suspend() { return {}; }
		final_awaiter final_suspend() noexcept
		{
			return{};
		}
		void return_void() {}
		//T return_value { return result; }
		void unhandled_exception() {}
		struct final_awaiter
		{
			bool await_ready() noexcept
			{
				return false;
			}
			void await_resume() noexcept
			{}
			std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept
			{
				std::cout << "delay_task::promise_type\n\t::final_awaiter::await_suspend \n\t h's address is " << h.address() << std::endl;
				// final_awaiter::await_suspend is called when the execution of the
				// current coroutine (referred to by 'h') is about to finish.
				// If the current coroutine was resumed by another coroutine via
				// co_await get_task(), a handle to that coroutine has been stored
				// as h.promise().previous. In that case, return the handle to resume
				// the previous coroutine.
				// Otherwise, return noop_coroutine(), whose resumption does nothing.

				auto previous = h.promise().previous;
				if (previous)
				{
					std::cout << "delay_task::promise_type\n\t::final_awaiter::await_suspend \n\t previous's address is " << previous.address() << std::endl;
					return previous;
				}
				else
				{
					return std::noop_coroutine();
				}
			}

		};
		static VOID CALLBACK TimerCallback(
			_Inout_     PTP_CALLBACK_INSTANCE Instance,
			_Inout_opt_ PVOID                 Context,
			_Inout_     PTP_TIMER             Timer
		)
		{
			CloseThreadpoolTimer(Timer);
			std::cout << "Timer expires, in thread (" << std::this_thread::get_id() << ")." << std::endl;

			auto promise = (promise_type*) Context;
		#if false
			//promise->result = 1;
			if (promise != nullptr && promise->previous)
				promise->previous.resume();
		#else
			if (promise != nullptr)
			{
				auto h = std::coroutine_handle<promise_type>::from_promise(*promise);
				h.resume();
			}

		#endif

		}

	};


	//awaitable

	struct delay_awaitable
	{
		//std::chrono::milliseconds duration;
		delay_task::handle m_handle;
		bool await_ready()
		{
			return false;
		}
		void await_suspend(std::coroutine_handle<> h)
		{
			std::cout << "delay_task::delay_awaitable\n\t::await_suspend \n\t h's address is " << h.address() << std::endl;
			m_handle.promise().previous = h;
			auto timer = CreateThreadpoolTimer(
				delay_task::promise_type::TimerCallback,
				&m_handle.promise(), nullptr);
			if (timer == nullptr)
			{
				throw new std::runtime_error("timer created failure.");
			}
			auto duration = m_handle.promise().duration;
			auto count = std::chrono::nanoseconds(duration).count() / 100 * -1;
			FILETIME ft;
			memcpy(&ft, &count, sizeof(INT64));
			SetThreadpoolTimer(timer, &ft, 0, 0);
		}
		//T await_resume()
		void await_resume()
		{
			//return m_handle.promise().result;
		}
	};

	delay_awaitable operator co_await()
	{
		using namespace std::chrono_literals;
		return delay_awaitable {m_handle};
	}

	//co_await a awaitable(as-is).
	//return delay_awaitable {duration};

};


delay_task delay(std::chrono::milliseconds duration)
{
	co_return;
}


struct task
{
	struct promise_type;
	using handle = std::coroutine_handle<promise_type>;
	handle m_handle;
	task(std::coroutine_handle<promise_type> h) : m_handle(h) {
		//std::cout << "task with handled(" << m_handle.address() << ") was created." << std::endl;
	}
	task(task&& t) = delete;
	~task() {
		//std::cout << "task with handled(" << m_handle.address() << ") was deleted." << std::endl;
		if (m_handle)
			m_handle.destroy();
	}
	struct promise_type
	{
		std::promise<void> wait;
		task get_return_object()
		{
			return task {std::coroutine_handle<promise_type>::from_promise(*this)};
		}
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept
		{
			wait.set_value();
			return {};
		}
		void return_void() {
		}
		void unhandled_exception() {}
	};


	void operator()()
	{
		//由于promise.initial_suspend()返回suspend_always。
		//故需要最外层调用()重载函数来调用下方的resume()。
		//然后就会执行a_coroutine_func()的代码了。
		m_handle.resume();
		m_handle.promise().wait.get_future().wait();

	}
};



task a_coroutine_func()
{
	using namespace std::chrono_literals;
	for (int i = 1; i < 4; ++i)
	{
	#if true
		auto d = delay(std::chrono::seconds(i));
		std::cout << "Delay for "<< i <<" seconds."<< std::endl;
		co_await d;
		std::cout << "a_coroutine_func() resumed in thread (" << std::this_thread::get_id() << ")." << std::endl;
	#endif

	}
	co_return;
}

int main(int argc, char* argv[])
{
	std::cout << "main thread(" << std::this_thread::get_id() << ")." << std::endl;
	auto coro = a_coroutine_func();
	coro();
	std::cout << "coro() returned in thread(" << std::this_thread::get_id() << ")." << std::endl;
}