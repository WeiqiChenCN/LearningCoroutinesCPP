#include <concepts>
#include <optional>
#include <coroutine>

namespace weiqi {

    struct empty_task {
        struct promise_type;
        struct awaiter_type;
        using handle = std::coroutine_handle<promise_type>;
        handle m_coroutine;
        explicit empty_task(const handle coroutine) :
            m_coroutine{ coroutine }
        {}
        struct promise_type
        {
            std::coroutine_handle<> m_previous;
            empty_task get_return_object()
            {
                return empty_task{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {}
        };
    };

}