// coroutine_traits.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// If this program fails to compile due to missing -fcoroutines flag
// required by GCC, try it at https://godbolt.org/z/b5zKax

#include <chrono>
#include <coroutine>
#include <exception>
#include <future>
#include <iostream>
#include <thread>
#include <type_traits>

// Enable the use of std::future<T> as a coroutine type
// by using a std::promise<T> as the promise type.
template <typename T, typename... Args>
requires(!std::is_void_v<T> && !std::is_reference_v<T>)
struct std::coroutine_traits<std::future<T>, Args...>
{
	~coroutine_traits()
	{
		std::cout << "~coroutine_traits() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
	}
	struct promise_type : std::promise<T>
	{
		std::future<T> get_return_object() noexcept
		{
			std::cout << "  promise_type::get_return_object() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			//std::cout << "promise_type with promise(" << this << ")'s get_return_object was invoked." << std::endl;
			return this->get_future();
		}

		~promise_type()
		{
			std::cout << "  ~promise_type() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
		}

		std::suspend_never initial_suspend() const noexcept { return {}; }
		std::suspend_never final_suspend() const noexcept { return {}; }

		void return_value(const T& value)
			noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			std::cout << "  promise_type::return_value(const T& value) was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			this->set_value(value);
		}
		void return_value(T&& value)
			noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			std::cout << "  promise_type::return_value(T&& value) was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			this->set_value(std::move(value));
		}
		void unhandled_exception() noexcept
		{
			this->set_exception(std::current_exception());
		}
	};
};

// Allow co_await'ing std::future<T> and std::future<void>
// by naively spawning a new thread for each co_await.
template <typename T>
auto operator co_await(std::future<T> future) noexcept
requires(!std::is_reference_v<T>)
{
	struct awaiter : std::future<T>
	{
		~awaiter()
		{
			std::cout << "pointer of this in ~awaiter():" << this << std::endl;
			std::cout << "~awaiter() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
		}
		bool await_ready() const noexcept
		{
			std::cout << "pointer of this in await_ready():" << this << std::endl;
			std::cout << "    awaiter::await_ready() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			//std::cout << "awaiter(" << this << ")'s await_ready was invoked." << std::endl;
			using namespace std::chrono_literals;
			return this->wait_for(0s) != std::future_status::timeout;
		}
		void await_suspend(std::coroutine_handle<> cont) const
		{
			std::cout << "pointer of cont.address():" << cont.address() << std::endl;
			std::cout << "    awaiter::await_suspend() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			//std::cout << "awaiter(" << this << ")'s await_suspend was invoked." << std::endl;
			std::thread t(
				[this, cont] {
					std::cout << "      a thread id(" << std::this_thread::get_id() << ") started." << std::endl;
					this->wait();
					std::cout << "      a thread id(" << std::this_thread::get_id() << ") call cont().resume()." << std::endl;
					cont.resume();
				}
			);
			std::cout << "    awaiter::await_suspend() swap a thread id(" << t.get_id() << ")." << std::endl;
			t.detach();
		}
		T await_resume()
		{
			std::cout << "    awaiter::await_resume() was invoked by thread id(" << std::this_thread::get_id() << ")." << std::endl;
			//std::cout << "awaiter(" << this << ")'s await_resume was invoked." << std::endl;
			return this->get();
		}
	};
	return awaiter {std::move(future)};
}

// Utilize the infrastructure we have established.
std::future<int> compute()
{
	//once this func is invoked, promise.get_return_object() will be invoked.
	using namespace std::chrono_literals;
	std::cout << "  right before first suspend point." << std::endl;
	//co_await match operator co_await, then create awaiter
	int a = co_await std::async
	([] {
		std::cout << "=>sleep 1000ms in thread id(" << std::this_thread::get_id() << ")." << std::endl;
		std::this_thread::sleep_for(1000ms);
		std::cout << "=>finished sleeping 1000ms in thread id(" << std::this_thread::get_id() << ")." << std::endl;
		return 6;
	 });
	//co_await match operator co_await, then create another awaiter
	int b = co_await std::async
	([] {
		std::cout << "=>sleep 2000ms in thread id(" << std::this_thread::get_id() << ")." << std::endl;
		std::this_thread::sleep_for(2000ms);
		std::cout << "=>finished sleeping 2000ms in thread id(" << std::this_thread::get_id() << ")." << std::endl;
		return 7;
	 });
	co_return a * b;
}

int main()
{
	std::cout << "main thread id(" << std::this_thread::get_id() << ") started." << std::endl;
	auto start = std::chrono::system_clock::now();
#if true
	std::cout << "Before a corotuine." << std::endl;
	auto ret = compute();
	std::cout << "After the corotuine." << std::endl;
	std::cout << "main thread id(" << std::this_thread::get_id() << ") blocked at ret.get()." << std::endl;
	std::cout << ret.get() << '\n'; //you must call get() to block current thread, 
#else

#endif
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;
	//std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
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
