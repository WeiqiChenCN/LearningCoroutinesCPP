// task.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <thread>
#include <iostream>
#include <coroutine>

//Reference: https://en.cppreference.com/w/cpp/language/coroutines

namespace weiqi {
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
        std::cout << "the body of 'a_coroutine_function' starts executing..." << std::endl;
        co_return;
    }
}

int main()
{
    std::cout << "Hello World!\n";
    auto x = weiqi::a_coroutine_function();
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
