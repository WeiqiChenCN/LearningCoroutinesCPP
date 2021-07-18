// loop_in_coroutine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <concepts>
#include <optional>
#include <coroutine>

#include <iostream>

namespace weiqi {

	struct task {
		struct promise_type;
		struct awaiter_type;
		using handle = std::coroutine_handle<promise_type>;
		handle m_coroutine;
		explicit task(const handle coroutine) :
			m_coroutine{ coroutine }
		{}
		~task() 
		{
		}
		task(const task&) = delete;
		task(task&& other) = delete;

		task& operator = (const task&) = delete;
		task& operator = (task&& other) = delete;

		auto operator co_await()
		{
			return awaiter_type {};
		}

		//the promise object, manipulated from inside the coroutine.
		//The coroutine submits its result or exception through
		//this object.
		struct promise_type
		{
			std::coroutine_handle<> m_previous;
			~promise_type() {}
			task get_return_object()
			{
				return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
			}
			std::suspend_never initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			void return_void() {}
			void unhandled_exception() {}

		};

		struct awaiter_type {

			bool await_ready() {
				return false;
			}
			auto await_suspend(handle handle) {
				handle.resume();
			}
			auto await_resume() { return; }

		};
	};

}

weiqi::task co_func2() {
	std::cout << "Hello from co_func2()";
	co_return;
}

weiqi::task co_func1() {
	int i = 0;
	while (i++ < 2) {
		co_await co_func2();
	}
	co_return;
}

int main()
{
	co_func2();
	std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
