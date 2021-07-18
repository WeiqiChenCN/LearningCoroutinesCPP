#include <concepts>
#include <optional>
#include <coroutine>

#include "empty_task.h"

//Reference: https://en.cppreference.com/w/cpp/coroutine/coroutine_handle
//Reference: https://en.cppreference.com/w/cpp/language/coroutines

namespace weiqi
{
	empty_task empty_func() 
	{
		co_return;
	}

	template<typename T>
	struct task
	{
		struct promise_type;
		using Handle = std::coroutine_handle<promise_type>;
		explicit task(const Handle coroutine) :
			m_coroutine{ coroutine }
		{}
#pragma region other constructor and delete constructor
		task() = default;
		~task() {
			if (m_coroutine) {
				m_coroutine.destroy();
			}
		}
		task(const task&) = delete;
		task& operator = (const task&) = delete;
		task(task&& other) noexcept :
			m_coroutine{ other.m_coroutine }
		{
			other.m_coroutine = {};
		}
		task& operator=(task&& other) noexcept {
			if (this != &other) {
				if (m_coroutine) {
					m_coroutine.destroy();
				}
				m_coroutine = other.m_coroutine;
				other.m_coroutine = {};
			}
			return *this;
		}
#pragma endregion
		struct promise_type {
			std::optional<T> current_value;
			task get_return_object()
			{
				return task{ Handle::from_promise(*this) };
			}
			static std::suspend_always initial_suspend() noexcept
			{
				return {};
			}
			static std::suspend_always final_suspend() noexcept
			{
				return {};
			}
			std::suspend_always yield_value(T value) noexcept
			{
				current_value = value;
				return {};
			}
			void return_void() {}
			// Disallow co_await in this coroutine body.
			template<typename _>
			std::suspend_never await_transform(_&&) = delete;
			[[noreturn]]
			static void unhandled_exception()
			{
				throw;
			}
		};
		Handle m_coroutine = nullptr;

		struct Iter
		{
			Handle m_coroutine;
			void operator++()
			{
				m_coroutine.resume();
			}
			const T& operator*() const
			{
				return *m_coroutine.promise().current_value;
			}
			bool operator==(std::default_sentinel_t) const
			{
				return !m_coroutine || m_coroutine.done();
			}
			explicit Iter(const Handle coroutine)
				: m_coroutine{ coroutine }
			{}
		};

		Iter begin()
		{
			if (m_coroutine)
			{
				m_coroutine.resume();
			}
			return Iter{ m_coroutine };
		}
		std::default_sentinel_t end()
		{
			return{};
		}
	};
	
	template<std::integral T>
	task<T> fibonacci_sequence(T end)
	{
		T first = 1;
		T second = 1;
		if (first <= end)
			co_yield first; //<--	When a coroutine reaches a suspension point 
							//		the return object obtained earlier is returned 
							//		to the caller/resumer, after implicit conversion
							//		to the return type of the coroutine, if necessary.
		if (second <= end)
			co_yield second;

		int third;
		while ((third = first + second) <= end) {
			first = second;
			second = third;
			co_yield third;
		}
		
		//co_await empty_func();
	}
}
