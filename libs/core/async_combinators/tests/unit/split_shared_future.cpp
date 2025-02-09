//  Copyright (c) 2016-2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/local/future.hpp>
#include <hpx/local/init.hpp>
#include <hpx/local/thread.hpp>
#include <hpx/modules/testing.hpp>

#include <array>
#include <chrono>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
hpx::tuple<> make_tuple0_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return hpx::make_tuple();
}

void test_split_future0()
{
    hpx::lcos::local::futures_factory<hpx::tuple<>()> pt(make_tuple0_slowly);
    pt.post();

    hpx::tuple<hpx::future<void>> result =
        hpx::split_future(hpx::shared_future<hpx::tuple<>>(pt.get_future()));

    hpx::get<0>(result).get();
}

///////////////////////////////////////////////////////////////////////////////
hpx::tuple<int> make_tuple1_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return hpx::make_tuple(42);
}

void test_split_future1()
{
    hpx::lcos::local::futures_factory<hpx::tuple<int>()> pt(make_tuple1_slowly);
    pt.post();

    hpx::tuple<hpx::future<int>> result =
        hpx::split_future(hpx::shared_future<hpx::tuple<int>>(pt.get_future()));

    HPX_TEST_EQ(hpx::get<0>(result).get(), 42);
}

///////////////////////////////////////////////////////////////////////////////
hpx::tuple<int, int> make_tuple2_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return hpx::make_tuple(42, 43);
}

void test_split_future2()
{
    hpx::lcos::local::futures_factory<hpx::tuple<int, int>()> pt(
        make_tuple2_slowly);
    pt.post();

    hpx::tuple<hpx::future<int>, hpx::future<int>> result = hpx::split_future(
        hpx::shared_future<hpx::tuple<int, int>>(pt.get_future()));

    HPX_TEST_EQ(hpx::get<0>(result).get(), 42);
    HPX_TEST_EQ(hpx::get<1>(result).get(), 43);
}

///////////////////////////////////////////////////////////////////////////////
hpx::tuple<int, int, int> make_tuple3_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return hpx::make_tuple(42, 43, 44);
}

void test_split_future3()
{
    hpx::lcos::local::futures_factory<hpx::tuple<int, int, int>()> pt(
        make_tuple3_slowly);
    pt.post();

    hpx::tuple<hpx::future<int>, hpx::future<int>, hpx::future<int>> result =
        hpx::split_future(
            hpx::shared_future<hpx::tuple<int, int, int>>(pt.get_future()));

    HPX_TEST_EQ(hpx::get<0>(result).get(), 42);
    HPX_TEST_EQ(hpx::get<1>(result).get(), 43);
    HPX_TEST_EQ(hpx::get<2>(result).get(), 44);
}

///////////////////////////////////////////////////////////////////////////////
std::pair<int, int> make_pair_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::make_pair(42, 43);
}

void test_split_future_pair()
{
    hpx::lcos::local::futures_factory<std::pair<int, int>()> pt(
        make_pair_slowly);
    pt.post();

    std::pair<hpx::future<int>, hpx::future<int>> result = hpx::split_future(
        hpx::shared_future<std::pair<int, int>>(pt.get_future()));

    HPX_TEST_EQ(result.first.get(), 42);
    HPX_TEST_EQ(result.second.get(), 43);
}

///////////////////////////////////////////////////////////////////////////////
std::array<int, 0> make_array0_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::array<int, 0>();
}

void test_split_future_array0()
{
    hpx::lcos::local::futures_factory<std::array<int, 0>()> pt(
        make_array0_slowly);
    pt.post();

    std::array<hpx::future<void>, 1> result =
        hpx::split_future(pt.get_future());

    result[0].get();
}

///////////////////////////////////////////////////////////////////////////////
std::array<int, 3> make_array_slowly()
{
    hpx::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::array<int, 3>{{42, 43, 44}};
}

void test_split_future_array()
{
    hpx::lcos::local::futures_factory<std::array<int, 3>()> pt(
        make_array_slowly);
    pt.post();

    std::array<hpx::future<int>, 3> result = hpx::split_future(pt.get_future());

    HPX_TEST_EQ(result[0].get(), 42);
    HPX_TEST_EQ(result[1].get(), 43);
    HPX_TEST_EQ(result[2].get(), 44);
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main()
{
    test_split_future0();
    test_split_future1();
    test_split_future2();
    test_split_future3();

    test_split_future_pair();

    test_split_future_array0();
    test_split_future_array();

    hpx::local::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    // We force this test to use several threads by default.
    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    // Initialize and run HPX
    hpx::local::init_params init_args;
    init_args.cfg = cfg;

    return hpx::local::init(hpx_main, argc, argv, init_args);
}
