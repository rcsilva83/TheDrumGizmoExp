# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.31.6)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(hugin-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[/usr/bin/git]==])
set(GIT_VERSION_STRING [==[2.53.0]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[/usr/bin/git;2.53.0]==]
)


include(ExternalProject)
ExternalProject_Add(hugin-populate
                     "GIT_REPOSITORY" "git://git.drumgizmo.org/hugin.git" "GIT_TAG" "3e2e1589dc50bd944cfd63d58a846921966224af"
                    SOURCE_DIR          "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/hugin/7fd996e4a78915a575ac6e60badae23c37e15b24"
                    BINARY_DIR          "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/hugin-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


