// fibonacci_sequence.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <concepts>
#include <iostream>

//Reference: https://www.boost.org/doc/libs/1_76_0/libs/coroutine2/ \
                     doc/html/coroutine2/coroutine/asymmetric.html
#if false
typedef boost::coroutines2::coroutine<int>   coro_t;

coro_t::pull_type source(
	[&](coro_t::push_type& sink) {
		int first = 1, second = 1;
		sink(first);
		sink(second);
		for (int i = 0; i < 8; ++i) {
			int third = first + second;
			first = second;
			second = third;
			sink(third);
		}
	});

for (auto i : source)
std::cout << i << " ";

//output:
//1 1 2 3 5 8 13 21 34 55
#endif


#include "fibonacci_sequence.h"

struct task {
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;
    Handle m_coroutine;
    explicit task(const Handle coroutine) :
        m_coroutine{ coroutine }
    {}
    struct promise_type {
        task get_return_object()
        {
            return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}

        void unhandled_exception() {}
    };
};

task a_coroutine_function() {
    //co_await weiqi::fibonacci_sequence(10);
    co_return;
}

int main()
{
	//auto t = weiqi::task<int>();
	for (int i : weiqi::fibonacci_sequence(100)) {
		std::cout << i << " ";
	}

	


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
