cmake_minimum_required(VERSION 3.16)

project(DrumGizmo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(DG_DEFAULT_INPUT_OSS OFF)
set(DG_DEFAULT_OUTPUT_OSS OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(DG_DEFAULT_INPUT_OSS ON)
	set(DG_DEFAULT_OUTPUT_OSS ON)
endif()

option(DG_WITH_DEBUG "Enable debug logging and debug warnings" OFF)
option(DG_ENABLE_CLI "Build CLI binaries" ON)
option(DG_ENABLE_DGVALIDATOR "Build dgvalidator" ON)
option(DG_WITH_NLS "Build with NLS support" OFF)

set(DG_NUM_CHANNELS 16 CACHE STRING "Number of output channels")
set_property(CACHE DG_NUM_CHANNELS PROPERTY STRINGS 8 16 24 32)

set(DG_SSE_LEVEL "auto" CACHE STRING "SSE level: auto, 0, 1, 2, 3")
set_property(CACHE DG_SSE_LEVEL PROPERTY STRINGS auto 0 1 2 3)

option(DG_ENABLE_INPUT_DUMMY "Enable dummy input engine" ON)
option(DG_ENABLE_INPUT_TEST "Enable test input engine" ON)
option(DG_ENABLE_INPUT_JACKMIDI "Enable JACK MIDI input engine" ON)
option(DG_ENABLE_INPUT_ALSAMIDI "Enable ALSA MIDI input engine" ON)
option(DG_ENABLE_INPUT_MIDIFILE "Enable SMF MIDI file input engine" ON)
option(DG_ENABLE_INPUT_OSSMIDI "Enable OSS MIDI input engine" ${DG_DEFAULT_INPUT_OSS})

option(DG_ENABLE_OUTPUT_DUMMY "Enable dummy output engine" ON)
option(DG_ENABLE_OUTPUT_ALSA "Enable ALSA output engine" ON)
option(DG_ENABLE_OUTPUT_WAVFILE "Enable wavfile output engine" ON)
option(DG_ENABLE_OUTPUT_JACKAUDIO "Enable JACK output engine" ON)
option(DG_ENABLE_OUTPUT_OSS "Enable OSS output engine" ${DG_DEFAULT_OUTPUT_OSS})

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile>=1.0.20)

set(DRUMGIZMO_VERSION "0.0.0")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.h" DG_VERSION_LINE REGEX "^#define VERSION \"[0-9.]+\"$")
if(DG_VERSION_LINE)
	string(REGEX REPLACE "^#define VERSION \"([0-9.]+)\"$" "\\1" DRUMGIZMO_VERSION "${DG_VERSION_LINE}")
endif()

set(DG_JACK_REQUIRED OFF)
if(DG_ENABLE_INPUT_JACKMIDI OR DG_ENABLE_OUTPUT_JACKAUDIO)
	set(DG_JACK_REQUIRED ON)
endif()

set(DG_ALSA_REQUIRED OFF)
if(DG_ENABLE_INPUT_ALSAMIDI OR DG_ENABLE_OUTPUT_ALSA)
	set(DG_ALSA_REQUIRED ON)
endif()

if(DG_ENABLE_INPUT_MIDIFILE)
	pkg_check_modules(SMF REQUIRED IMPORTED_TARGET smf>=1.2)
endif()

if(DG_JACK_REQUIRED)
	pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack>=0.120.1)
endif()

if(DG_ALSA_REQUIRED)
	pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa>=1.0.18)
endif()

check_function_exists(wordexp HAVE_WORDEXP)

if(DG_ENABLE_INPUT_OSSMIDI OR DG_ENABLE_OUTPUT_OSS)
	check_include_file(sys/soundcard.h DG_HAVE_SYS_SOUNDCARD_H)
	if(NOT DG_HAVE_SYS_SOUNDCARD_H)
		message(FATAL_ERROR "OSS support requested but sys/soundcard.h was not found")
	endif()
endif()

set(DG_COMMON_COMPILE_DEFINITIONS)
if(DG_WITH_DEBUG)
	list(APPEND DG_COMMON_COMPILE_OPTIONS -Wall -Werror -Wextra -g)
else()
	list(APPEND DG_COMMON_COMPILE_DEFINITIONS DISABLE_HUGIN)
endif()

check_cxx_compiler_flag("-ffloat-store" DG_HAVE_FFLOAT_STORE)
if(DG_HAVE_FFLOAT_STORE)
	list(APPEND DG_COMMON_COMPILE_OPTIONS -ffloat-store)
endif()

if(DG_SSE_LEVEL STREQUAL "1")
	list(APPEND DG_COMMON_COMPILE_DEFINITIONS SSE)
	list(APPEND DG_COMMON_COMPILE_OPTIONS -msse)
elseif(DG_SSE_LEVEL STREQUAL "2")
	list(APPEND DG_COMMON_COMPILE_DEFINITIONS SSE)
	list(APPEND DG_COMMON_COMPILE_OPTIONS -msse -msse2)
elseif(DG_SSE_LEVEL STREQUAL "3")
	list(APPEND DG_COMMON_COMPILE_DEFINITIONS SSE)
	list(APPEND DG_COMMON_COMPILE_OPTIONS -msse -msse2 -msse3)
elseif(DG_SSE_LEVEL STREQUAL "auto")
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
		list(APPEND DG_COMMON_COMPILE_DEFINITIONS SSE)
		list(APPEND DG_COMMON_COMPILE_OPTIONS -msse)
	endif()
endif()

set(WITH_NLS ${DG_WITH_NLS})
set(HAVE_INPUT_DUMMY ${DG_ENABLE_INPUT_DUMMY})
set(HAVE_INPUT_TEST ${DG_ENABLE_INPUT_TEST})
set(HAVE_INPUT_MIDIFILE ${DG_ENABLE_INPUT_MIDIFILE})
set(HAVE_INPUT_JACKMIDI ${DG_ENABLE_INPUT_JACKMIDI})
set(HAVE_INPUT_ALSAMIDI ${DG_ENABLE_INPUT_ALSAMIDI})
set(HAVE_INPUT_OSS ${DG_ENABLE_INPUT_OSSMIDI})
set(HAVE_INPUT_OSSMIDI ${DG_ENABLE_INPUT_OSSMIDI})

set(HAVE_OUTPUT_DUMMY ${DG_ENABLE_OUTPUT_DUMMY})
set(HAVE_OUTPUT_WAVFILE ${DG_ENABLE_OUTPUT_WAVFILE})
set(HAVE_OUTPUT_ALSA ${DG_ENABLE_OUTPUT_ALSA})
set(HAVE_OUTPUT_JACKAUDIO ${DG_ENABLE_OUTPUT_JACKAUDIO})
set(HAVE_OUTPUT_OSS ${DG_ENABLE_OUTPUT_OSS})

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

add_library(dg_zr STATIC
	src/zrwrapper.cc
	zita-resampler/libs/resampler.cc
	zita-resampler/libs/resampler-table.cc
)
target_compile_definitions(dg_zr PRIVATE _USE_MATH_DEFINES)
target_compile_options(dg_zr PRIVATE -ffast-math)
target_include_directories(dg_zr PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/zita-resampler/libs"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(dg_zr PRIVATE Threads::Threads)

add_library(dg_pugi STATIC
	pugixml/src/pugixml.cpp
)
target_include_directories(dg_pugi PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src")

add_library(dg_core STATIC
	src/audiocachefile.cc
	src/audiocache.cc
	src/audiocacheeventhandler.cc
	src/audiocacheidmanager.cc
	src/audioinputenginemidi.cc
	src/audiofile.cc
	src/bytesizeparser.cc
	src/channel.cc
	src/channelmixer.cc
	src/configfile.cc
	src/configparser.cc
	src/directory.cc
	src/domloader.cc
	src/dgxmlparser.cc
	src/drumgizmo.cc
	src/drumgizmoconf.cc
	src/drumkit.cc
	src/drumkitloader.cc
	src/events.cc
	src/events_ds.cc
	src/inputprocessor.cc
	src/instrument.cc
	src/latencyfilter.cc
	src/midimapparser.cc
	src/midimapper.cc
	src/path.cc
	src/powerlist.cc
	src/powermap.cc
	src/powermapfilter.cc
	src/random.cc
	src/sample.cc
	src/sample_selection.cc
	src/sem.cc
	src/staminafilter.cc
	src/thread.cc
	src/velocityfilter.cc
	src/versionstr.cc
)
target_include_directories(dg_core PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/hugin"
	"${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src"
	"${CMAKE_CURRENT_SOURCE_DIR}/zita-resampler/libs"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
target_compile_definitions(dg_core PRIVATE ${DG_COMMON_COMPILE_DEFINITIONS})
target_compile_options(dg_core PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
target_link_libraries(dg_core PUBLIC PkgConfig::SNDFILE Threads::Threads dg_zr dg_pugi)

check_library_exists(rt clock_gettime "" DG_HAVE_LIBRT)
if(DG_HAVE_LIBRT)
	target_link_libraries(dg_core PUBLIC rt)
endif()

check_library_exists(atomic __atomic_fetch_add_8 "" DG_HAVE_LIBATOMIC)
if(DG_HAVE_LIBATOMIC)
	target_link_libraries(dg_core PUBLIC atomic)
endif()

if(DG_ENABLE_CLI)
	add_library(dg_hugin STATIC
		hugin/hugin.c
		hugin/hugin_filter.c
	)
	target_compile_definitions(dg_hugin PRIVATE WITH_HUG_MUTEX WITH_HUG_FILTER)
	target_include_directories(dg_hugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/hugin")

	set(DG_CLI_COMMON_SOURCES
		drumgizmo/enginefactory.cc
	)
	set(DG_CLI_COMMON_DEFINITIONS WITH_HUG_MUTEX WITH_HUG_FILTER)

	if(DG_ENABLE_INPUT_DUMMY)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/inputdummy.cc)
	endif()

	if(DG_ENABLE_INPUT_TEST)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/test.cc)
	endif()

	if(DG_ENABLE_INPUT_MIDIFILE)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/midifile.cc)
	endif()

	if(DG_ENABLE_INPUT_JACKMIDI)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/jackmidi.cc)
	endif()

	if(DG_ENABLE_INPUT_ALSAMIDI)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/alsamidi.cc)
	endif()

	if(DG_ENABLE_INPUT_OSSMIDI)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/input/ossmidi.cc)
	endif()

	if(DG_ENABLE_OUTPUT_DUMMY)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/output/outputdummy.cc)
	endif()

	if(DG_ENABLE_OUTPUT_WAVFILE)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/output/wavfile.cc)
	endif()

	if(DG_ENABLE_OUTPUT_ALSA)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/output/alsa.cc)
	endif()

	if(DG_ENABLE_OUTPUT_JACKAUDIO)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/output/jackaudio.cc)
	endif()

	if(DG_ENABLE_OUTPUT_OSS)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/output/oss.cc)
	endif()

	if(DG_ENABLE_INPUT_JACKMIDI OR DG_ENABLE_OUTPUT_JACKAUDIO)
		list(APPEND DG_CLI_COMMON_SOURCES drumgizmo/jackclient.cc)
	endif()

	set(DG_CLI_COMMON_LINK_LIBRARIES dg_core dg_hugin)
	if(DG_ENABLE_INPUT_MIDIFILE)
		list(APPEND DG_CLI_COMMON_LINK_LIBRARIES PkgConfig::SMF)
	endif()
	if(DG_JACK_REQUIRED)
		list(APPEND DG_CLI_COMMON_LINK_LIBRARIES PkgConfig::JACK)
	endif()
	if(DG_ALSA_REQUIRED)
		list(APPEND DG_CLI_COMMON_LINK_LIBRARIES PkgConfig::ALSA)
	endif()

	add_executable(drumgizmo
		drumgizmo/drumgizmoc.cc
		${DG_CLI_COMMON_SOURCES}
	)
	target_compile_definitions(drumgizmo PRIVATE
		${DG_COMMON_COMPILE_DEFINITIONS}
		${DG_CLI_COMMON_DEFINITIONS}
	)
	target_compile_options(drumgizmo PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
	target_include_directories(drumgizmo PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${CMAKE_CURRENT_SOURCE_DIR}/getoptpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/hugin"
		"${CMAKE_CURRENT_BINARY_DIR}"
	)
	target_link_libraries(drumgizmo PRIVATE ${DG_CLI_COMMON_LINK_LIBRARIES})
	install(TARGETS drumgizmo RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

	if(DG_ENABLE_DGVALIDATOR)
		add_executable(dgvalidator
			drumgizmo/dgvalidator.cc
			${DG_CLI_COMMON_SOURCES}
			dggui/lodepng/lodepng.cpp
			dggui/image.cc
			dggui/resource.cc
			dggui/colour.cc
		)
		target_compile_definitions(dgvalidator PRIVATE
			${DG_COMMON_COMPILE_DEFINITIONS}
			${DG_CLI_COMMON_DEFINITIONS}
			LODEPNG_NO_COMPILE_ENCODER
			LODEPNG_NO_COMPILE_DISK
			LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
			LODEPNG_NO_COMPILE_ERROR_TEXT
			LODEPNG_NO_COMPILE_CPP
		)
		target_compile_options(dgvalidator PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
		target_include_directories(dgvalidator PRIVATE
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${CMAKE_CURRENT_SOURCE_DIR}/getoptpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/hugin"
			"${CMAKE_CURRENT_BINARY_DIR}"
		)
		target_link_libraries(dgvalidator PRIVATE ${DG_CLI_COMMON_LINK_LIBRARIES})
		install(TARGETS dgvalidator RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
endif()
