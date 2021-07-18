// language.coroutines.example.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

auto switch_to_new_thread(std::jthread& out)
{
	struct awaitable
	{
		std::jthread* p_out;
		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> h)
		{
			std::jthread& out = *p_out;
			if (out.joinable())
				throw std::runtime_error("Output jthread parameter not empty");
			out = std::jthread([h]
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1000ms);
				h.resume();
			});
			// Potential undefined behavior: accessing potentially destroyed *this
			// since h.resume() will be invoked in jthread above soon.
			// std::cout << "New thread ID: " << p_out->get_id() << '\n';
			std::cout << "New thread ID: " << out.get_id() << '\n'; // this is OK
		}
		void await_resume() {}
	};
	return awaitable {&out};
}

struct task
{
	task()
	{
		std::cout << "task() in this(" << this << ")." << std::endl;

	}
	~task()
	{
		std::cout << "~task() in this(" << this << ")." << std::endl;
	}
	struct promise_type
	{
		~promise_type()
		{
			std::cout << "~promise_type in thread id(" << std::this_thread::get_id() << ")" << std::endl;
		}
		task get_return_object() { return {}; }
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() {}
	};
};

task resuming_on_new_thread(std::jthread& out)
{
	std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
	co_await switch_to_new_thread(out);
	// awaiter destroyed here
	std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}

int main()
{
	std::jthread out;
	resuming_on_new_thread(out);
	std::cout << "main exit()." << std::endl;
	//since jthread automatically rejoins on destruction,
	//when jthread  invoke h.resume(), h.resume() invoke awaitable::await_resume()
	//and then await_resume() will contine the execution of the coroutine.
	//when resuming_on_new_thread return, the operation of joining jthread finished
	//then main thread exit.
#if false
	//if you call exit(0), programs will exit witnout destructing jthread.
	exit(0);
#endif
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
