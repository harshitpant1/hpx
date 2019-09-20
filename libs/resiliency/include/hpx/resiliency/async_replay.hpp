//  Copyright (c) 2019 National Technology & Engineering Solutions of Sandia,
//                     LLC (NTESS).
//  Copyright (c) 2018-2019 Hartmut Kaiser
//  Copyright (c) 2018-2019 Adrian Serio
//  Copyright (c) 2019 Nikunj Gupta
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_RESILIENCY_ASYNC_REPLAY_HPP_2019_FEB_04_0442PM)
#define HPX_RESILIENCY_ASYNC_REPLAY_HPP_2019_FEB_04_0442PM

#include <hpx/resiliency/config.hpp>

#include <hpx/async.hpp>
#include <hpx/dataflow.hpp>
#include <hpx/datastructures/detail/pack.hpp>
#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace hpx { namespace resiliency {

    ///////////////////////////////////////////////////////////////////////////
    struct HPX_ALWAYS_EXPORT abort_replay_exception : std::exception
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        ///////////////////////////////////////////////////////////////////////
        struct replay_validator
        {
            template <typename T>
            bool operator()(T&& result) const
            {
                return true;
            }
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename Future>
        std::exception_ptr rethrow_on_abort_replay(Future& f)
        {
            std::exception_ptr ex;
            try
            {
                f.get();
            }
            catch (abort_replay_exception const&)
            {
                throw;
            }
            catch (...)
            {
                ex = std::current_exception();
            }
            return ex;
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Result, typename Pred, typename F, typename Tuple>
        struct async_replay_helper
          : std::enable_shared_from_this<
                async_replay_helper<Result, Pred, F, Tuple>>
        {
            template <typename Pred_, typename F_, typename Tuple_>
            async_replay_helper(Pred_&& pred, F_&& f, Tuple_&& tuple)
              : pred_(std::forward<Pred_>(pred))
              , f_(std::forward<F_>(f))
              , t_(std::forward<Tuple_>(tuple))
            {
            }

            template <std::size_t... Is>
            hpx::future<Result> invoke(
                hpx::util::detail::pack_c<std::size_t, Is...>)
            {
                return hpx::async(f_, std::get<Is>(t_)...);
            }

            hpx::future<Result> call(std::size_t n)
            {
                // launch given function asynchronously
                hpx::future<Result> f =
                    invoke(hpx::util::detail::make_index_pack<
                        std::tuple_size<Tuple>::value>{});

                // attach a continuation that will relaunch the task, if
                // necessary
                auto this_ = this->shared_from_this();
                return f.then(hpx::launch::sync,
                    [HPX_CAPTURE_MOVE(this_), n](hpx::future<Result>&& f) {
                        if (f.has_exception())
                        {
                            // rethrow abort_replay_exception, if caught
                            auto ex = rethrow_on_abort_replay(f);

                            // execute the task again if an error occurred and
                            // this was not the last attempt
                            if (n != 0)
                            {
                                return this_->call(n - 1);
                            }

                            // rethrow exception if the number of replays has
                            // been exhausted
                            std::rethrow_exception(ex);
                        }

                        auto&& result = f.get();

                        if (!hpx::util::invoke(this_->pred_, result))
                        {
                            // execute the task again if an error occurred and
                            // this was not the last attempt
                            if (n != 0)
                            {
                                return this_->call(n - 1);
                            }

                            // throw aborting exception as attempts were
                            // exhausted
                            throw abort_replay_exception();
                        }

                        if (n != 0)
                        {
                            // return result
                            return hpx::make_ready_future(std::move(result));
                        }

                        // throw aborting exception as attempts were
                        // exhausted
                        throw abort_replay_exception();
                    });
            }

            Pred pred_;
            F f_;
            Tuple t_;
        };

        template <typename Result, typename Pred, typename F, typename... Ts>
        std::shared_ptr<async_replay_helper<Result,
            typename std::decay<Pred>::type, typename std::decay<F>::type,
            std::tuple<typename std::decay<Ts>::type...>>>
        make_async_replay_helper(Pred&& pred, F&& f, Ts&&... ts)
        {
            using tuple_type = std::tuple<typename std::decay<Ts>::type...>;

            using return_type = async_replay_helper<Result,
                typename std::decay<Pred>::type, typename std::decay<F>::type,
                std::tuple<typename std::decay<Ts>::type...>>;

            return std::make_shared<return_type>(std::forward<Pred>(pred),
                std::forward<F>(f), std::make_tuple(std::forward<Ts>(ts)...));
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    /// Asynchronously launch given function \a f. Verify the result of
    /// those invocations using the given predicate \a pred. Repeat launching
    /// on error exactly \a n times (except if abort_replay_exception is thrown).
    template <typename Pred, typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    async_replay_validate(std::size_t n, Pred&& pred, F&& f, Ts&&... ts)
    {
        using result_type =
            typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type;

        auto helper = detail::make_async_replay_helper<result_type>(
            std::forward<Pred>(pred), std::forward<F>(f),
            std::forward<Ts>(ts)...);

        return helper->call(n);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Asynchronously launch given function \a f. Repeat launching
    /// on error exactly \a n times (except if abort_replay_exception is thrown).
    template <typename F, typename... Ts>
    hpx::future<
        typename hpx::util::detail::invoke_deferred_result<F, Ts...>::type>
    async_replay(std::size_t n, F&& f, Ts&&... ts)
    {
        return async_replay_validate(n, detail::replay_validator{},
            std::forward<F>(f), std::forward<Ts>(ts)...);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Functional version of \a hpx::resiliency::async_replay
    namespace functional {

        struct async_replay_validate
        {
            template <typename Pred, typename F, typename... Ts>
            auto operator()(std::size_t n, Pred&& pred, F&& f, Ts&&... ts) const
                -> decltype(hpx::resiliency::async_replay_validate(n,
                    std::forward<Pred>(pred), std::forward<F>(f),
                    std::forward<Ts>(ts)...))
            {
                return hpx::resiliency::async_replay_validate(n,
                    std::forward<Pred>(pred), std::forward<F>(f),
                    std::forward<Ts>(ts)...);
            }
        };

        struct async_replay
        {
            template <typename F, typename... Ts>
            auto operator()(std::size_t n, F&& f, Ts&&... ts) const
                -> decltype(hpx::resiliency::async_replay(
                    n, std::forward<F>(f), std::forward<Ts>(ts)...))
            {
                return hpx::resiliency::async_replay(
                    n, std::forward<F>(f), std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional
}}       // namespace hpx::resiliency

#endif
