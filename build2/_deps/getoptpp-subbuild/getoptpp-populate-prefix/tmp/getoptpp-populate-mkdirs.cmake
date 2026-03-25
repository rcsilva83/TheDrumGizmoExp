# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/getoptpp/bbae31606e455f8990d3bbfaad963ebffbfd836a")
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/getoptpp/bbae31606e455f8990d3bbfaad963ebffbfd836a")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-build"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/tmp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/src/getoptpp-populate-stamp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/src"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/src/getoptpp-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/src/getoptpp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/getoptpp-subbuild/getoptpp-populate-prefix/src/getoptpp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
