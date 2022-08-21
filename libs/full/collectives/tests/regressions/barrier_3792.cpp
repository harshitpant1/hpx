// Copyright (c) 2019 Weile Wei
// Copyright (c) 2019 Maxwell Reeser
// Copyright (c) 2019 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
void run_barrier_test(std::vector<size_t> locs)
{
    std::string barrier_name =
        "/loc_list/barrier" + std::to_string(locs[0]) + std::to_string(locs[1]);
    hpx::distributed::barrier b(barrier_name,
        hpx::collectives::num_sites_arg(locs.size()),
        hpx::collectives::this_site_arg(hpx::get_locality_id()));
    b.wait();
}

int hpx_main()
{
    std::cout << "Hello world from locality " << hpx::get_locality_id()
              << std::endl;
    std::vector<size_t> locs_0{0, 1};
    run_barrier_test(locs_0);
    std::vector<size_t> locs_1{0, 2};
    run_barrier_test(locs_1);
    std::vector<size_t> locs_2{1, 2};
    run_barrier_test(locs_2);
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // We force hpx_main to run on all processes
    std::vector<std::string> const cfg = {"hpx.run_hpx_main!=1"};
    hpx::init_params init_args;
    init_args.cfg = cfg;

    return hpx::init(argc, argv, init_args);
}
#endif
