#!/bin/bash -l

# Copyright (c) 2020 ETH Zurich
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set -eux

src_dir="$(pwd)"
build_dir="${src_dir}/build/${configuration_name_with_build_type}"

rm -rf "${build_dir}"
mkdir -p "${build_dir}"

source ${src_dir}/.jenkins/lsu/env-common.sh
source ${src_dir}/.jenkins/lsu/env-${configuration_name}.sh

ulimit -l unlimited

set +e
ctest \
    --verbose \
    -S ${src_dir}/.jenkins/lsu/ctest.cmake \
    -DCTEST_CONFIGURE_EXTRA_OPTIONS="${configure_extra_options}" \
    -DCTEST_BUILD_CONFIGURATION_NAME="${configuration_name_with_build_type}" \
    -DCTEST_SOURCE_DIRECTORY="${src_dir}" \
    -DCTEST_BINARY_DIRECTORY="${build_dir}"

if [[ "${install_hpx}" ]]; then
    install_root="/work/jenkins/install"
    install_dir_suffix_master="hpx-${configuration_name_with_build_type}"
    install_dir_suffix_commit="${install_dir_suffix_master}-${GIT_COMMIT}"
    install_dir_master="${install_root}/${install_dir_suffix_master}"
    install_dir_commit="${install_root}/${install_dir_suffix_commit}"

    # Install the current build into a directory suffixed by the commit hash
    cmake "${build_dir}" -DCMAKE_INSTALL_PREFIX="${install_dir_commit}"
    cmake --build "${build_dir}" --target install

    # Link the current build to a directory without the hash
    rm -f "${install_dir_master}"
    ln -s "${install_dir_commit}" "${install_dir_master}"

    # Delete all but the 10 newest builds
    builds_to_keep=10
    rm -rf $(ls -d --sort=time --reverse ${install_dir_master}-* | head --lines=-${builds_to_keep})

    module save "${install_dir_suffix_commit}"
fi
set -e

# Things went wrong by default
ctest_exit_code=$?
file_errors=1
configure_errors=1
build_errors=1
test_errors=1
if [[ -f ${build_dir}/Testing/TAG ]]; then
    file_errors=0
    tag="$(head -n 1 ${build_dir}/Testing/TAG)"

    if [[ -f "${build_dir}/Testing/${tag}/Configure.xml" ]]; then
        configure_errors=$(grep '<Error>' "${build_dir}/Testing/${tag}/Configure.xml" | wc -l)
    fi

    if [[ -f "${build_dir}/Testing/${tag}/Build.xml" ]]; then
        build_errors=$(grep '<Error>' "${build_dir}/Testing/${tag}/Build.xml" | wc -l)
    fi

    if [[ -f "${build_dir}/Testing/${tag}/Test.xml" ]]; then
        test_errors=$(grep '<Test Status=\"failed\">' "${build_dir}/Testing/${tag}/Test.xml" | wc -l)
    fi
fi
ctest_status=$(( ctest_exit_code + file_errors + configure_errors + build_errors + test_errors ))

echo "${ctest_status}" > "jenkins-hpx-${configuration_name_with_build_type}-ctest-status.txt"
exit $ctest_status
