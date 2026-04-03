# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/smf/e4a400205483c54b02ad66fcfe7aaa42b3f76bf3")
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/smf/e4a400205483c54b02ad66fcfe7aaa42b3f76bf3")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-build"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/tmp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/src/smf-populate-stamp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/src"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/src/smf-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/src/smf-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/smf-subbuild/smf-populate-prefix/src/smf-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
