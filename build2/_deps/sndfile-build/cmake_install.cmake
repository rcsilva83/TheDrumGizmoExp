# Install script for directory: /home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/libsndfile.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/include/sndfile.h"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/include/sndfile.hh"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile/SndFileTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile/SndFileTargets.cmake"
         "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/CMakeFiles/Export/5c71f72976042dd672d3a20ad1898c82/SndFileTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile/SndFileTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile/SndFileTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile" TYPE FILE FILES "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/CMakeFiles/Export/5c71f72976042dd672d3a20ad1898c82/SndFileTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile" TYPE FILE FILES "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/CMakeFiles/Export/5c71f72976042dd672d3a20ad1898c82/SndFileTargets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile" TYPE FILE RENAME "SndFileConfig.cmake" FILES "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/SndFileConfig2.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/SndFile" TYPE FILE FILES "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/SndFileConfigVersion.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/DrumGizmo" TYPE FILE FILES
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/index.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/libsndfile.jpg"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/libsndfile.css"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/print.css"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/api.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/command.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/bugs.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/formats.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/sndfile_info.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/new_file_type_howto.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/win32.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/FAQ.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/lists.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/embedded_files.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/octave.md"
    "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/cpm-cache/sndfile/80dfa406d3d9afee05174ef3bef06ff15c7b7f2c/docs/tutorial.md"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/TheDrumGizmoExp/TheDrumGizmoExp/build2/_deps/sndfile-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
