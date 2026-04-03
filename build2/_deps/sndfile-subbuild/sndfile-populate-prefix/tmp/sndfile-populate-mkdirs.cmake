# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c")
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/tmp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/src/sndfile-populate-stamp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/src"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/src/sndfile-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/src/sndfile-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-subbuild/sndfile-populate-prefix/src/sndfile-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
