# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/zitaresampler/bc12097f0d098d59651f8ea2b9ef5f6bed57fce8")
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/zitaresampler/bc12097f0d098d59651f8ea2b9ef5f6bed57fce8")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-build"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/tmp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/src/zitaresampler-populate-stamp"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/src"
  "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/src/zitaresampler-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/src/zitaresampler-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/zitaresampler-subbuild/zitaresampler-populate-prefix/src/zitaresampler-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
