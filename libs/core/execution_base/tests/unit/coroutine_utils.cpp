//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config/coroutines_support.hpp>
#include <hpx/execution/algorithms/just.hpp>
#include <hpx/execution/algorithms/sync_wait.hpp>
#include <hpx/execution_base/completion_signatures.hpp>
#include <hpx/execution_base/coroutine_utils.hpp>
#include <hpx/modules/testing.hpp>

#include "coroutine_task.hpp"

#include <exception>
#include <iostream>
#include <utility>

template <typename Error, typename... Values>
auto signature_all(Error, Values...)
    -> hpx::execution::experimental::completion_signatures<
        hpx::execution::experimental::set_value_t(Values...),
        hpx::execution::experimental::set_error_t(Error),
        hpx::execution::experimental::set_stopped_t()>
{
    return {};
}

template <typename Signatures>
struct non_awaitable_sender
{
    using completion_signatures = Signatures;
};

struct promise
{
    hpx::coro::coroutine_handle<promise> get_return_object()
    {
        return {hpx::coro::coroutine_handle<promise>::from_promise(*this)};
    }
    hpx::coro::suspend_always initial_suspend() noexcept
    {
        return {};
    }
    hpx::coro::suspend_always final_suspend() noexcept
    {
        return {};
    }
    void return_void() {}
    void unhandled_exception() {}
};

template <typename Awaiter>
struct awaitable_sender_1
{
    Awaiter operator co_await()
    {
        return Awaiter{};
    }
};

struct awaiter
{
    bool await_ready()
    {
        return true;
    }
    bool await_suspend(hpx::coro::coroutine_handle<>)
    {
        return false;
    }
    bool await_resume()
    {
        return false;
    }
};

using dependent = hpx::execution::experimental::dependent_completion_signatures<
    hpx::execution::experimental::no_env>;

struct recv_set_value
{
    friend void tag_invoke(hpx::execution::experimental::set_value_t,
        recv_set_value,
        decltype(std::declval<dependent>().await_ready())) noexcept
    {
    }
    friend void tag_invoke(
        hpx::execution::experimental::set_stopped_t, recv_set_value) noexcept
    {
    }
    friend void tag_invoke(hpx::execution::experimental::set_error_t,
        recv_set_value, std::exception_ptr) noexcept
    {
    }
    friend dependent tag_invoke(
        hpx::execution::experimental::get_env_t, const recv_set_value&) noexcept
    {
        return {};
    }
};

template <class T>
T& unmove(T&& t)
{
    return t;
}

template <typename S1, typename S2,
    typename = std::enable_if_t<hpx::execution::experimental::is_sender_v<S1> &&
        hpx::execution::experimental::is_sender_v<S2>>>
task<int> async_answer(S1 s1, S2 s2)
{
    // Senders are implicitly awaitable (in this coroutine type):
    co_await(S2 &&) s2;
    co_return co_await(S1 &&) s1;
}

int main()
{
    using namespace hpx::execution::experimental;

    {
        static_assert(
            std::is_same_v<single_sender_value_t<non_awaitable_sender<decltype(
                               signature_all(std::exception_ptr(), int()))>>,
                int>);
        static_assert(
            std::is_same_v<single_sender_value_t<non_awaitable_sender<decltype(
                               signature_all(std::exception_ptr()))>>,
                void>);
    }

    // single sender value
    {
        static_assert(
            std::is_same_v<single_sender_value_t<awaitable_sender_1<awaiter>>,
                bool>);
        static_assert(
            std::is_same_v<single_sender_value_t<
                               awaitable_sender_1<hpx::coro::suspend_always>>,
                void>);
    }

    // connect awaitable
    {
        static_assert(std::is_same_v<decltype(connect_awaitable(
                                         awaitable_sender_1<awaiter>{},
                                         recv_set_value{})),
            operation_t<recv_set_value>>);

        static_assert(
            std::is_same_v<decltype(connect(awaitable_sender_1<awaiter>{},
                               recv_set_value{})),
                operation_t<recv_set_value>>);
    }

    // Promise base
    {
        static_assert(is_awaitable_v<awaitable_sender_1<awaiter>,
            promise_t<recv_set_value>>);
    }

    // Operation base
    {
        static_assert(is_operation_state_v<operation_t<recv_set_value>>);
    }

    // Connect result type
    {
        static_assert(std::is_same_v<
            connect_result_t<awaitable_sender_1<awaiter>, recv_set_value>,
            operation_t<recv_set_value>>);
    }

    // As awaitable
    {
        static_assert(is_awaitable_v<decltype(as_awaitable(
                awaitable_sender_1<awaiter>{}, unmove(::promise{})))>);
        static_assert(
            std::is_same_v<decltype(as_awaitable(awaitable_sender_1<awaiter>{},
                               unmove(::promise{}))),
                awaitable_sender_1<awaiter>&&>);
    }

    try
    {
        // Awaitables are implicitly senders:
        auto i = hpx::this_thread::experimental::sync_wait(
            async_answer(hpx::execution::experimental::just(42),
                hpx::execution::experimental::just()))
                     .value();
        std::cout << "The answer is " << hpx::get<0>(i) << '\n';
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    return hpx::util::report_errors();
}
