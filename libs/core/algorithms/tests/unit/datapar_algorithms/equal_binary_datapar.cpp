//  Copyright (c) 2014-2020 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/include/datapar.hpp>
#include <hpx/local/init.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "../algorithms/equal_binary_tests.hpp"

////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_equal_binary1()
{
    using namespace hpx::execution;

    test_equal_binary1(simd, IteratorTag());
    test_equal_binary1(par_simd, IteratorTag());

    test_equal_binary1_async(simd(task), IteratorTag());
    test_equal_binary1_async(par_simd(task), IteratorTag());
}

void equal_binary_test1()
{
    test_equal_binary1<std::random_access_iterator_tag>();
    test_equal_binary1<std::forward_iterator_tag>();
}

////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_equal_binary2()
{
    using namespace hpx::execution;

    test_equal_binary2(simd, IteratorTag());
    test_equal_binary2(par_simd, IteratorTag());

    test_equal_binary2_async(simd(task), IteratorTag());
    test_equal_binary2_async(par_simd(task), IteratorTag());
}

void equal_binary_test2()
{
    test_equal_binary2<std::random_access_iterator_tag>();
    test_equal_binary2<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    gen.seed(seed);

    equal_binary_test1();
    equal_binary_test2();
    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace hpx::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    // Initialize and run HPX
    hpx::local::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    init_args.cfg = cfg;

    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv, init_args), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
