//  Copyright (c) 2007-2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/affinity/parse_affinity_options.hpp>
#include <hpx/local/thread.hpp>
#include <hpx/modules/testing.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// The affinity masks this test is verifying the results against are specific
// to a particular machine. If you enable this option you might see a lot of
// test failures, which is expected.
// The bit masks in the tests below are assuming a 12 core system (with
// hyper threading), with 2 NUMA nodes (2 sockets), 6 cores each.
// #define VERIFY_AFFINITY_MASKS

///////////////////////////////////////////////////////////////////////////////
namespace hpx::threads::detail {
    std::ostream& operator<<(std::ostream& os, spec_type const& data)
    {
        os << spec_type::type_name(data.type_);
        for (std::size_t i : data.index_bounds_)
        {
            os << "," << i;
        }
        return os;
    }
}    // namespace hpx::threads::detail

namespace test {

    using hpx::threads::detail::spec_type;

    struct data_good_thread
    {
        spec_type thread;
        spec_type socket;
        spec_type core;
        spec_type pu;
    };

    struct data_good
    {
        std::string option_;
        std::size_t threads_;
        data_good_thread t[4];
        std::uint64_t masks[4];
    };

    //  Test cases implemented below:
    //
    //   thread:0-1=socket:0
    //   thread:0-1=socket:0-1
    //   thread:0-1=numanode:0
    //   thread:0-1=numanode:0-1
    //   thread:0-1=core:0
    //   thread:0-1=core:0-1
    //   thread:0-1=core:0.pu:0
    //   thread:0-1=core:0.pu:0-1
    //   thread:0-1=pu:0
    //   thread:0-1=pu:0-1
    //   thread:0-1=socket:0.core:0
    //   thread:0-1=socket:1.core:0-1
    //   thread:0-1=numanode:0.core:0
    //   thread:0-1=numanode:1.core:0-1
    //   thread:0-1=socket:1.core:1.pu:0
    //   thread:0-1=socket:1.core:1.pu:0-1
    //   thread:0-1=numanode:1.core:1.pu:0
    //   thread:0-1=numanode:1.core:1.pu:0-1
    //   thread:0-1=socket:1.core:0-1.pu:1
    //   thread:0-1=numanode:1.core:0-1.pu:1
    //   thread:0-1=socket:0-1.core:1.pu:1
    //   thread:0-1=numanode:0-1.core:1.pu:1
    //   thread:0-1=socket:0-1.pu:0
    //   thread:0-1=numanode:0-1.pu:0
    //   thread:0-1=socket:0.pu:0
    //   thread:0-1=socket:0.pu:0-1
    //   thread:0-1=numanode:0.pu:0
    //   thread:0-1=numanode:0.pu:0-1
    //   thread:0-1=socket:0.core:all.pu:0

#if defined(__GNUC__) && __GNUC__ < 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

    // clang-format off
    data_good data[] =
    {
        {   "thread:0=socket:0;thread:1=socket:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000fff, 0x000fff }
        },
        {   "thread:0-1=socket:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0x000fff }
        },
        {   "thread:0,1=socket:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0x000fff }
        },

        {   "thread:0=socket:0;thread:1=socket:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000fff, 0xfff000 }
        },
        {   "thread:0-1=socket:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0xfff000 }
        },
        {   "thread:0,1=socket:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::socket, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0xfff000 }
        },

        {   "thread:0=numanode:0;thread:1=numanode:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000fff, 0x000fff }
        },
        {   "thread:0-1=numanode:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0x000fff }
        },
        {   "thread:0,1=numanode:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0x000fff }
        },

        {   "thread:0=numanode:0;thread:1=numanode:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000fff, 0xfff000 }
        },
        {   "thread:0-1=numanode:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0xfff000 }
        },
        {   "thread:0,1=numanode:0,1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::numanode, 0, 1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000fff, 0xfff000 }
        },

        {   "thread:0=core:0;thread:1=core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000003, 0x000003 }
        },
        {   "thread:0-1=core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x000003 }
        },
        {   "thread:0,1=core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x000003 }
        },

        {   "thread:0=core:0;thread:1=core:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000003, 0x00000c }
        },
        {   "thread:0-1=core:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x00000c }
        },
        {   "thread:0,1=core:0,1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, 1),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x00000c }
        },

        {   "thread:0=core:1.pu:0;thread:1=core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000004, 0x000004 }
        },
        {   "thread:0-1=core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000004, 0x000004 }
        },

        {   "thread:0=core:1.pu:0;thread:1=core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000004, 0x000008 }
        },
        {   "thread:0-1=core:1.pu:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x000004, 0x000008 }
        },

        {   "thread:0=pu:0;thread:1=pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000001, 0x000001 }
        },
        {   "thread:0-1=pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000001 }
        },

        {   "thread:0=pu:0;thread:1=pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000001, 0x000002 }
        },
        {   "thread:0-1=pu:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x000001, 0x000002 }
        },

        {   "thread:0=socket:0.core:0;thread:1=socket:0.core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000003, 0x000003 }
        },
        {   "thread:0-1=socket:0.core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x000003 }
        },

        {   "thread:0=socket:1.core:0;thread:1=socket:1.core:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x003000, 0x00c000 }
        },
        {   "thread:0-1=socket:1.core:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x003000, 0x00c000 }
        },

        {   "thread:0=numanode:0.core:0;thread:1=numanode:0.core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x000003, 0x000003 }
        },
        {   "thread:0-1=numanode:0.core:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x000003, 0x000003 }
        },

        {   "thread:0=numanode:1.core:0;thread:1=numanode:1.core:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::unknown)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::unknown)
                }
            },
            { 0x003000, 0x00c000 }
        },
        {   "thread:0-1=numanode:1.core:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::unknown)
                }, data_good_thread()
            },
            { 0x003000, 0x00c000 }
        },

        {   "thread:0=socket:1.core:0.pu:1;thread:1=socket:1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x002000, 0x008000 }
        },
        {   "thread:0-1=socket:1.core:0-1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::pu, 1, 0)
                }, data_good_thread()
            },
            { 0x002000, 0x008000 }
        },

        {   "thread:0=socket:1.core:1.pu:0;thread:1=socket:1.core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x004000, 0x004000 }
        },
        {   "thread:0-1=socket:1.core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x004000, 0x004000 }
        },

        {   "thread:0=socket:1.core:1.pu:0;thread:1=socket:1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x004000, 0x008000 }
        },
        {   "thread:0-1=socket:1.core:1.pu:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x004000, 0x008000 }
        },

        {   "thread:0=numanode:1.core:1.pu:0;thread:1=numanode:1.core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x004000, 0x004000 }
        },
        {   "thread:0-1=numanode:1.core:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x004000, 0x004000 }
        },

        {   "thread:0=numanode:1.core:0.pu:1;thread:1=numanode:1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 0, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }, {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x002000, 0x008000 }
        },
        {   "thread:0-1=numanode:1.core:0-1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::pu, 1, 0)
                }, data_good_thread()
            },
            { 0x002000, 0x008000 }
        },

        {   "thread:0-1=socket:0.core:all.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::core, spec_type::all_entities(), 0),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000004 }
        },

        {   "thread:0=socket:0.core:1.pu:1;thread:1=socket:1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000008, 0x008000 }
        },
        {   "thread:0-1=socket:0-1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, -1),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }, data_good_thread()
            },
            { 0x000008, 0x008000 }
        },

        {   "thread:0=numanode:0.core:1.pu:1;thread:1=numanode:1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000008, 0x008000 }
        },
        {   "thread:0-1=numanode:0-1.core:1.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, -1),
                    spec_type(spec_type::type::core, 1, 0),
                    spec_type(spec_type::type::pu, 1, 0)
                }, data_good_thread()
            },
            { 0x000008, 0x008000 }
        },

        {   "thread:0=socket:0.pu:0;thread:1=socket:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000001, 0x001000 }
        },
        {   "thread:0-1=socket:0-1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x001000 }
        },

        {   "thread:0=numanode:0.pu:0;thread:1=numanode:1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 1, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000001, 0x001000 }
        },
        {   "thread:0-1=numanode:0-1.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, -1),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x001000 }
        },

        {   "thread:0=socket:0.pu:0;thread:1=socket:0.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000001, 0x000001 }
        },
        {   "thread:0-1=socket:0.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000001 }
        },

        {   "thread:0=socket:0.pu:0;thread:1=socket:0.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000001, 0x000002 }
        },
        {   "thread:0-1=socket:0.pu:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::socket, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x000001, 0x000002 }
        },

        {   "thread:0=numanode:0.pu:0;thread:1=numanode:0.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }
            },
            { 0x000001, 0x000001 }
        },
        {   "thread:0-1=numanode:0.pu:0", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000001 }
        },

        {   "thread:0=numanode:0.pu:0;thread:1=numanode:0.pu:1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                },
                {
                    spec_type(spec_type::type::thread, 1, 0),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 1, 0)
                }
            },
            { 0x000001, 0x000002 }
        },
        {   "thread:0-1=numanode:0.pu:0-1", 2,
            {
                {
                    spec_type(spec_type::type::thread, 0, -1),
                    spec_type(spec_type::type::numanode, 0, 0),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x000001, 0x000002 }
        },

        // test cases derived from #2312
        {   "thread:0-3=core:0-1.pu:0-1", 4,
            {
                {
                    spec_type(spec_type::type::thread, 0, -3),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, -1),
                    spec_type(spec_type::type::pu, 0, -1)
                }, data_good_thread()
            },
            { 0x000001, 0x000002, 0x000004, 0x000008 }
        },
        {   "thread:0-3=core:0-3.pu:0", 4,
            {
                {
                    spec_type(spec_type::type::thread, 0, -3),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::core, 0, -3),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000004, 0x0000010, 0x000040 }
        },
        {   "thread:0-3=pu:0", 4,
            {
                {
                    spec_type(spec_type::type::thread, 0, -3),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::unknown),
                    spec_type(spec_type::type::pu, 0, 0)
                }, data_good_thread()
            },
            { 0x000001, 0x000001, 0x000001, 0x000001 }
        },

        { "", 0,  {data_good_thread(), data_good_thread()}, {0,0} }
    };
    // clang-format on

#if defined(__GNUC__) && __GNUC__ < 5
#pragma GCC diagnostic pop
#endif

    void good_testing(data_good const* t, char const* const options)
    {
        hpx::threads::detail::mappings_type mappings;
        hpx::error_code ec;
        hpx::threads::detail::parse_mappings(options, mappings, ec);
        HPX_TEST(!ec);

        int i = 0;

        HPX_TEST_EQ(mappings.which(), 1);
        if (mappings.which() == 1)
        {
            hpx::threads::detail::mappings_spec_type mappings_specs(
                boost::get<hpx::threads::detail::mappings_spec_type>(mappings));

            for (hpx::threads::detail::full_mapping_type const& m :
                mappings_specs)
            {
                HPX_TEST_EQ(t->t[i].thread, m.first);
                HPX_TEST_EQ(m.second.size(), 3u);
                if (m.second.size() == 3u)
                {
                    HPX_TEST_EQ(t->t[i].socket, m.second[0]);
                    HPX_TEST_EQ(t->t[i].core, m.second[1]);
                    HPX_TEST_EQ(t->t[i].pu, m.second[2]);
                }
                ++i;
            }
        }

#if defined(VERIFY_AFFINITY_MASKS)
        std::vector<hpx::threads::mask_type> affinities;
        affinities.resize(hpx::get_os_thread_count(), 0);
        hpx::threads::parse_affinity_options(options, affinities, ec);
        HPX_TEST(!ec);
        HPX_TEST_EQ(affinities.size(), t->threads_);
        HPX_TEST_EQ(std::count(affinities.begin(), affinities.end(), 0), 0);

        i = 0;
        for (hpx::threads::mask_type const& m : affinities)
        {
            HPX_TEST_EQ(t->masks[i], m);
            ++i;
        }
#endif
    }

    std::string replace_all(
        std::string str, char const* const what, char const* const with)
    {
        std::string::size_type p = str.find(what);
        if (p != std::string::npos)
        {
            std::size_t len = strnlen(what, 64);
            do
            {
                str.replace(p, len, with);
                p = str.find(what, p + len);
            } while (p != std::string::npos);
        }
        return str;
    }

    std::string shorten_options(std::string str)
    {
        str = replace_all(str, "thread", "t");
        str = replace_all(str, "socket", "s");
        str = replace_all(str, "numanode", "n");
        str = replace_all(str, "core", "c");
        return replace_all(str, "pu", "p");
    }

    void good()
    {
        for (data_good* t = data; !t->option_.empty(); ++t)
        {
            // test full length options
            good_testing(t, t->option_.c_str());

            // test shortened options
            std::string shortened_options(shorten_options(t->option_));
            good_testing(t, shortened_options.c_str());
        }
    }

    // clang-format off
    char const* const data_bad[] =
    {
        // wrong sequence
        "thread:0=pu:0.socket:0",
        "thread:0=pu:0.numanode:0",
        "thread:0=pu:0.core:0",
        "thread:0=core:0.socket:0",
        "thread:0=core:0.numanode:0",
        "thread:0=numanode:0.socket:0",
        // duplicates
        "thread:0=socket:0.socket:0",
        "thread:0=numanode:0.numanode:0",
        "thread:0=core:0.core:0",
        "thread:0=pu:0.pu:0",
        "thread:1=socket:0.numanode:0",
        // empty
        "thread:0=socket",
        "thread:0=numanode",
        "thread:0=core",
        "thread:0=pu",
        "thread=",
        "socket:0",
        "numanode:0",
        "core:0",
        "pu:0",
        nullptr
    };
    // clang-format on

    void bad()
    {
        int i = 0;
        hpx::error_code ec;
        for (char const* t = data_bad[0]; nullptr != t; t = data_bad[++i])
        {
            std::vector<hpx::threads::mask_type> affinities;
            hpx::threads::parse_affinity_options(t, affinities, ec);
            HPX_TEST(ec);
        }
    }
}    // namespace test

///////////////////////////////////////////////////////////////////////////////
int main()
{
    {
        test::good();
        test::bad();
    }

    return hpx::util::report_errors();
}
