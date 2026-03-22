#!/bin/bash
# This is a script to faciliate running single tests.
#
# Usage: ./run_test.sh <test_1> <test_2> ... <test_n>
# If no test string is passed then all tests are run.

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
repo_root=$(cd "${script_dir}/.." && pwd)
build_dir="${DG_BUILD_DIR:-${repo_root}/build}"

if [ ! -d "${build_dir}" ]; then
	echo "Build directory '${build_dir}' was not found. Configure and build first."
	exit 1
fi

if [[ $# == 0 ]]
then
	echo "======================"
	echo "All tests are now run."
	echo "======================"
	ctest --test-dir "${build_dir}" --output-on-failure
else
	for TST in "$@"
	do
		echo "========================="
		echo "The $TST test is now run."
		echo "========================="
		ctest --test-dir "${build_dir}" -R "^${TST}$" --output-on-failure
	done
fi
