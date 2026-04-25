cmake_minimum_required(VERSION 3.16)

project(DrumGizmo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Required for building shared libraries (LV2/VST plugins) from static libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake")

set(DG_HUGIN_INCLUDE_DIR "${DG_HUGIN_SOURCE_DIR}")
set(DG_ZITA_RESAMPLER_LIB_DIR "${DG_ZITA_RESAMPLER_SOURCE_DIR}/libs")
set(DG_GETOPTPP_INCLUDE_DIR "${DG_GETOPTPP_SOURCE_DIR}")
set(DG_LODEPNG_SOURCE_FILE "${DG_LODEPNG_SOURCE_DIR}/lodepng.cpp")
set(DG_PLUGINGIZMO_INCLUDE_DIR "${DG_PLUGINGIZMO_SOURCE_DIR}")
set(DG_PUGL_INCLUDE_DIR "${DG_PUGL_SOURCE_DIR}")

function(dg_assert_files_exist)
	foreach(dg_file IN LISTS ARGN)
		if(NOT EXISTS "${dg_file}")
			message(FATAL_ERROR "Required file was not found: ${dg_file}")
		endif()
	endforeach()
endfunction()

set(DG_DEFAULT_INPUT_OSS OFF)
set(DG_DEFAULT_OUTPUT_OSS OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(DG_DEFAULT_INPUT_OSS ON)
	set(DG_DEFAULT_OUTPUT_OSS ON)
endif()

option(DG_WITH_DEBUG "Enable debug logging and debug warnings" OFF)
option(DG_ENABLE_TESTS "Build unit tests" OFF)
option(DG_ENABLE_CLI "Build CLI binaries" ON)
option(DG_ENABLE_DGVALIDATOR "Build dgvalidator" ON)
option(DG_WITH_NLS "Build with NLS support" OFF)
option(DG_ENABLE_LV2 "Build the LV2 plugin" OFF)
option(DG_ENABLE_VST "Build the VST plugin" OFF)
option(DG_ENABLE_PLUGINGUI_TEST "Build the plugin GUI test app" OFF)
option(DG_ENABLE_GUI_TESTS "Build GUI-dependent unit tests" ON)

set(DG_NUM_CHANNELS 16 CACHE STRING "Number of output channels")
set_property(CACHE DG_NUM_CHANNELS PROPERTY STRINGS 8 16 24 32)

set(DG_SSE_LEVEL "auto" CACHE STRING "SSE level: auto, 0, 1, 2, 3")
set_property(CACHE DG_SSE_LEVEL PROPERTY STRINGS auto 0 1 2 3)

set(DG_GUI_BACKEND "auto" CACHE STRING "GUI backend: auto, x11, win32, cocoa, pugl-x11, pugl-win32, pugl-cocoa")
set_property(CACHE DG_GUI_BACKEND PROPERTY STRINGS auto x11 win32 cocoa pugl-x11 pugl-win32 pugl-cocoa)

set(DG_LV2_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/lv2" CACHE PATH "LV2 install root directory")
set(DG_VST_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/vst" CACHE PATH "VST install directory")
set(DG_LV2_PLUGIN_URI "http://drumgizmo.org/lv2" CACHE STRING "LV2 plugin URI")
set(DG_VST_SDK_PATH "" CACHE PATH "Path to VST SDK 2.4 source tree")

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
	pkg_check_modules(GLIB2 IMPORTED_TARGET glib-2.0>=2.2)
	if(NOT GLIB2_FOUND)
		message(STATUS "glib-2.0 not found, disabling DG_ENABLE_INPUT_MIDIFILE")
		set(DG_ENABLE_INPUT_MIDIFILE OFF)
	endif()
endif()

if(DG_JACK_REQUIRED)
	pkg_check_modules(JACK IMPORTED_TARGET jack>=0.120.1)
	if(NOT JACK_FOUND)
		message(STATUS "jack not found, disabling JACK MIDI and JACK audio")
		set(DG_ENABLE_INPUT_JACKMIDI OFF)
		set(DG_ENABLE_OUTPUT_JACKAUDIO OFF)
		set(DG_JACK_REQUIRED OFF)
	endif()
endif()

if(DG_ALSA_REQUIRED)
	pkg_check_modules(ALSA IMPORTED_TARGET alsa>=1.0.18)
	if(NOT ALSA_FOUND)
		message(STATUS "ALSA not found, disabling ALSA MIDI and ALSA audio")
		set(DG_ENABLE_INPUT_ALSAMIDI OFF)
		set(DG_ENABLE_OUTPUT_ALSA OFF)
		set(DG_ALSA_REQUIRED OFF)
	endif()
endif()

if(DG_ENABLE_TESTS AND DG_ENABLE_GUI_TESTS AND DG_GUI_BACKEND STREQUAL "auto"
	AND NOT DG_ENABLE_LV2 AND NOT DG_ENABLE_VST AND NOT DG_ENABLE_PLUGINGUI_TEST)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME MATCHES ".*BSD")
		pkg_check_modules(X11_PROBE QUIET IMPORTED_TARGET x11>=1.0)
		pkg_check_modules(XEXT_PROBE QUIET IMPORTED_TARGET xext>=1.0)
		if(NOT X11_PROBE_FOUND OR NOT XEXT_PROBE_FOUND)
			message(STATUS "x11/xext not found, disabling DG_ENABLE_GUI_TESTS")
			set(DG_ENABLE_GUI_TESTS OFF)
		endif()
	endif()
endif()

set(DG_NEED_PLUGIN_GUI OFF)
if(DG_ENABLE_LV2 OR DG_ENABLE_VST OR DG_ENABLE_PLUGINGUI_TEST OR (DG_ENABLE_TESTS AND DG_ENABLE_GUI_TESTS))
	set(DG_NEED_PLUGIN_GUI ON)
endif()

set(DG_SELECTED_GUI_BACKEND "${DG_GUI_BACKEND}")
if(DG_NEED_PLUGIN_GUI)
	if(DG_SELECTED_GUI_BACKEND STREQUAL "auto")
		if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
			set(DG_SELECTED_GUI_BACKEND "cocoa")
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME MATCHES ".*BSD")
			set(DG_SELECTED_GUI_BACKEND "x11")
		elseif(WIN32)
			set(DG_SELECTED_GUI_BACKEND "win32")
		else()
			message(FATAL_ERROR "Could not determine a GUI backend for this platform. Set DG_GUI_BACKEND explicitly.")
		endif()
	endif()

	set(DG_GUI_COMPILE_DEFINITIONS)
	set(DG_GUI_LINK_LIBRARIES)
	set(DG_DGGUI_BACKEND_SOURCES)
	set(DG_DGGUI_BACKEND_INCLUDE_DIRS)

	if(DG_SELECTED_GUI_BACKEND STREQUAL "x11")
		pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11>=1.0)
		pkg_check_modules(XEXT REQUIRED IMPORTED_TARGET xext>=1.0)
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_X11)
		list(APPEND DG_GUI_LINK_LIBRARIES PkgConfig::X11 PkgConfig::XEXT)
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_x11.cc)
	elseif(DG_SELECTED_GUI_BACKEND STREQUAL "win32")
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_WIN32)
		list(APPEND DG_GUI_LINK_LIBRARIES gdi32 setupapi ws2_32 comctl32)
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_win32.cc)
	elseif(DG_SELECTED_GUI_BACKEND STREQUAL "cocoa")
		enable_language(OBJC)
		enable_language(OBJCXX)
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_COCOA)
		list(APPEND DG_GUI_LINK_LIBRARIES "-framework Cocoa")
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_cocoa.mm)
	elseif(DG_SELECTED_GUI_BACKEND STREQUAL "pugl-x11")
		pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11>=1.0)
		pkg_check_modules(XEXT REQUIRED IMPORTED_TARGET xext>=1.0)
		find_package(OpenGL REQUIRED)
		dg_assert_files_exist("${DG_PUGL_SOURCE_DIR}/pugl/pugl_x11.c")
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_PUGL PUGL_HAVE_GL)
		list(APPEND DG_GUI_LINK_LIBRARIES OpenGL::GL PkgConfig::X11 PkgConfig::XEXT)
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_pugl.cc "${DG_PUGL_SOURCE_DIR}/pugl/pugl_x11.c")
		list(APPEND DG_DGGUI_BACKEND_INCLUDE_DIRS "${DG_PUGL_INCLUDE_DIR}")
	elseif(DG_SELECTED_GUI_BACKEND STREQUAL "pugl-win32")
		dg_assert_files_exist("${DG_PUGL_SOURCE_DIR}/pugl/pugl_win.cpp")
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_PUGL PUGL_HAVE_GL)
		list(APPEND DG_GUI_LINK_LIBRARIES opengl32 gdi32 user32 setupapi ws2_32 comctl32)
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_pugl.cc "${DG_PUGL_SOURCE_DIR}/pugl/pugl_win.cpp")
		list(APPEND DG_DGGUI_BACKEND_INCLUDE_DIRS "${DG_PUGL_INCLUDE_DIR}")
	elseif(DG_SELECTED_GUI_BACKEND STREQUAL "pugl-cocoa")
		enable_language(OBJC)
		enable_language(OBJCXX)
		find_package(OpenGL REQUIRED)
		dg_assert_files_exist("${DG_PUGL_SOURCE_DIR}/pugl/pugl_osx.m")
		list(APPEND DG_GUI_COMPILE_DEFINITIONS UI_PUGL PUGL_HAVE_GL)
		list(APPEND DG_GUI_LINK_LIBRARIES OpenGL::GL "-framework Cocoa")
		list(APPEND DG_DGGUI_BACKEND_SOURCES dggui/nativewindow_pugl.cc "${DG_PUGL_SOURCE_DIR}/pugl/pugl_osx.m")
		list(APPEND DG_DGGUI_BACKEND_INCLUDE_DIRS "${DG_PUGL_INCLUDE_DIR}")
	else()
		message(FATAL_ERROR "Unsupported DG_GUI_BACKEND value: ${DG_SELECTED_GUI_BACKEND}")
	endif()

	if(DG_ENABLE_LV2)
		dg_assert_files_exist(
			"${DG_PLUGINGIZMO_SOURCE_DIR}/pluginlv2.cc"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/midievent.cc"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/ttlgen.cc"
		)
		pkg_check_modules(LV2 REQUIRED IMPORTED_TARGET lv2>=1.0)
		pkg_check_modules(LILV REQUIRED IMPORTED_TARGET lilv-0)
		pkg_check_modules(SERD REQUIRED IMPORTED_TARGET serd-0)
		find_package(OpenSSL REQUIRED)
		check_include_file(dlfcn.h DG_HAVE_DLFCN_H)
		if(NOT DG_HAVE_DLFCN_H)
			message(FATAL_ERROR "LV2 support requested but dlfcn.h was not found")
		endif()

		set(DG_DL_LIBRARIES)
		check_library_exists(c dlopen "" DG_HAVE_DLOPEN_IN_LIBC)
		if(NOT DG_HAVE_DLOPEN_IN_LIBC)
			check_library_exists(dl dlopen "" DG_HAVE_DLOPEN_IN_LIBDL)
			if(NOT DG_HAVE_DLOPEN_IN_LIBDL)
				message(FATAL_ERROR "LV2 support requested but no library with dlopen was found")
			endif()
			list(APPEND DG_DL_LIBRARIES dl)
		endif()
	endif()

	if(DG_ENABLE_VST)
		dg_assert_files_exist(
			"${DG_PLUGINGIZMO_SOURCE_DIR}/pluginvst.cc"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/midievent.cc"
		)
		if(DG_VST_SDK_PATH STREQUAL "")
			message(FATAL_ERROR "VST support requested but DG_VST_SDK_PATH was not provided")
		endif()
		dg_assert_files_exist(
			"${DG_VST_SDK_PATH}/pluginterfaces/vst2.x/aeffect.h"
			"${DG_VST_SDK_PATH}/pluginterfaces/vst2.x/aeffectx.h"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffect.h"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffect.cpp"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffectx.h"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffectx.cpp"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/aeffeditor.h"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/vstplugmain.cpp"
		)
	endif()
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
	"${DG_ZITA_RESAMPLER_SOURCE_DIR}/libs/resampler.cc"
	"${DG_ZITA_RESAMPLER_SOURCE_DIR}/libs/resampler-table.cc"
)
target_compile_definitions(dg_zr PRIVATE _USE_MATH_DEFINES)
target_compile_options(dg_zr PRIVATE -ffast-math)
target_include_directories(dg_zr PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${DG_ZITA_RESAMPLER_LIB_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(dg_zr PRIVATE Threads::Threads)

add_library(dg_pugi ALIAS pugixml)

if(DG_ENABLE_INPUT_MIDIFILE)
	file(WRITE "${CMAKE_BINARY_DIR}/smf-config/config.h" "#define PACKAGE_VERSION \"${DG_SMF_PACKAGE_VERSION}\"\n")
	add_library(dg_smf STATIC
		"${DG_SMF_SOURCE_DIR}/src/smf.c"
		"${DG_SMF_SOURCE_DIR}/src/smf_decode.c"
		"${DG_SMF_SOURCE_DIR}/src/smf_load.c"
		"${DG_SMF_SOURCE_DIR}/src/smf_save.c"
		"${DG_SMF_SOURCE_DIR}/src/smf_tempo.c"
	)
	target_include_directories(dg_smf
		PUBLIC "${DG_SMF_SOURCE_DIR}/src"
		PRIVATE "${CMAKE_BINARY_DIR}/smf-config"
	)
	target_link_libraries(dg_smf PUBLIC PkgConfig::GLIB2)
endif()

add_library(dg_nls STATIC
	src/translation.cc
)
target_compile_definitions(dg_nls PRIVATE ${DG_COMMON_COMPILE_DEFINITIONS})
target_compile_options(dg_nls PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
target_include_directories(dg_nls PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${CMAKE_CURRENT_BINARY_DIR}"
)

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
	"${DG_HUGIN_INCLUDE_DIR}"
	"${DG_ZITA_RESAMPLER_LIB_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}"
)
target_compile_definitions(dg_core PRIVATE ${DG_COMMON_COMPILE_DEFINITIONS})
target_compile_options(dg_core PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
target_link_libraries(dg_core PUBLIC SndFile::sndfile Threads::Threads dg_zr dg_pugi)

dg_assert_files_exist("${DG_LODEPNG_SOURCE_FILE}")
add_library(dg_lodepng STATIC
	"${DG_LODEPNG_SOURCE_FILE}"
)
target_compile_definitions(dg_lodepng PUBLIC
	LODEPNG_NO_COMPILE_ENCODER
	LODEPNG_NO_COMPILE_DISK
	LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
	LODEPNG_NO_COMPILE_ERROR_TEXT
	LODEPNG_NO_COMPILE_CPP
)
target_compile_options(dg_lodepng PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
target_include_directories(dg_lodepng PUBLIC
	"${DG_LODEPNG_SOURCE_DIR}"
)

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
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${DG_HUGIN_SOURCE_DIR}/hugin_filter.c"
	)
	target_compile_definitions(dg_hugin PRIVATE WITH_HUG_MUTEX WITH_HUG_FILTER)
	target_include_directories(dg_hugin PUBLIC "${DG_HUGIN_INCLUDE_DIR}")

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
		list(APPEND DG_CLI_COMMON_LINK_LIBRARIES dg_smf)
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
		"${DG_GETOPTPP_INCLUDE_DIR}"
		"${DG_HUGIN_INCLUDE_DIR}"
		"${CMAKE_CURRENT_BINARY_DIR}"
	)
	target_link_libraries(drumgizmo PRIVATE ${DG_CLI_COMMON_LINK_LIBRARIES})
	install(TARGETS drumgizmo RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

	if(DG_ENABLE_DGVALIDATOR)
		add_executable(dgvalidator
			drumgizmo/dgvalidator.cc
			${DG_CLI_COMMON_SOURCES}
			dggui/image.cc
			dggui/resource.cc
			dggui/colour.cc
		)
		target_compile_definitions(dgvalidator PRIVATE
			${DG_COMMON_COMPILE_DEFINITIONS}
			${DG_CLI_COMMON_DEFINITIONS}
		)
		target_compile_options(dgvalidator PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
		target_include_directories(dgvalidator PRIVATE
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${DG_GETOPTPP_INCLUDE_DIR}"
			"${DG_HUGIN_INCLUDE_DIR}"
			"${CMAKE_CURRENT_BINARY_DIR}"
		)
		target_link_libraries(dgvalidator PRIVATE ${DG_CLI_COMMON_LINK_LIBRARIES} dg_lodepng)
		install(TARGETS dgvalidator RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
	endif()
endif()

if(DG_NEED_PLUGIN_GUI)
	dg_assert_files_exist(
		"${DG_GETOPTPP_SOURCE_DIR}/getoptpp.hpp"
	)

	add_executable(rcgen
		dggui/rcgentool.cc
	)
	target_include_directories(rcgen PRIVATE "${DG_GETOPTPP_INCLUDE_DIR}")

	set(DG_DGGUI_SOURCES
		dggui/button.cc
		dggui/button_base.cc
		dggui/checkbox.cc
		dggui/colour.cc
		dggui/combobox.cc
		dggui/dialog.cc
		dggui/eventhandler.cc
		dggui/font.cc
		dggui/frame.cc
		dggui/helpbutton.cc
		dggui/image.cc
		dggui/imagecache.cc
		dggui/knob.cc
		dggui/label.cc
		dggui/layout.cc
		dggui/led.cc
		dggui/lineedit.cc
		dggui/listbox.cc
		dggui/listboxbasic.cc
		dggui/listboxthin.cc
		dggui/painter.cc
		dggui/pixelbuffer.cc
		dggui/powerbutton.cc
		dggui/progressbar.cc
		dggui/rc_data.cc
		dggui/resource.cc
		dggui/scrollbar.cc
		dggui/slider.cc
		dggui/stackedwidget.cc
		dggui/tabbutton.cc
		dggui/tabwidget.cc
		dggui/textedit.cc
		dggui/texture.cc
		dggui/texturedbox.cc
		dggui/toggle.cc
		dggui/tooltip.cc
		dggui/uitranslation.cc
		dggui/utf8.cc
		dggui/verticalline.cc
		dggui/widget.cc
		dggui/window.cc
		${DG_DGGUI_BACKEND_SOURCES}
	)

	add_library(dg_dggui STATIC
		${DG_DGGUI_SOURCES}
	)
	target_compile_definitions(dg_dggui PRIVATE
		${DG_COMMON_COMPILE_DEFINITIONS}
		${DG_GUI_COMPILE_DEFINITIONS}
		WITH_HUG_MUTEX
	)
	target_compile_options(dg_dggui PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
	target_include_directories(dg_dggui PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${DG_HUGIN_INCLUDE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${CMAKE_CURRENT_BINARY_DIR}"
		${DG_DGGUI_BACKEND_INCLUDE_DIRS}
	)
	target_link_libraries(dg_dggui PUBLIC Threads::Threads ${DG_GUI_LINK_LIBRARIES} dg_lodepng)

	set(DG_PLUGINGUI_RESOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/bg.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/bypass_button.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/font.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/fontemboss.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/help_button.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/knob.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/logo.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/png_error"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/progress.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/pushbutton.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/sidebar.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/slider.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/stddev_horizontal.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/stddev_horizontal_disabled.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/stddev_vertical.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/stddev_vertical_disabled.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/switch_back_off.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/switch_back_on.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/switch_front.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/tab.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/thinlistbox.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/topbar.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/toplogo.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/vertline.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/resources/widget.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/ABOUT"
		"${CMAKE_CURRENT_SOURCE_DIR}/ABOUT.fr"
		"${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS"
		"${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS.fr"
		"${CMAKE_CURRENT_SOURCE_DIR}/BUGS"
		"${CMAKE_CURRENT_SOURCE_DIR}/BUGS.fr"
		"${CMAKE_CURRENT_SOURCE_DIR}/COPYING"
	)

	set(DG_NLS_RESOURCES)
	if(DG_WITH_NLS)
		find_program(MSGFMT_EXECUTABLE msgfmt)
		if(NOT MSGFMT_EXECUTABLE)
			message(FATAL_ERROR "DG_WITH_NLS is ON but msgfmt was not found")
		endif()

		file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugingui/locale")
		set(DG_NLS_RESOURCES
			"${CMAKE_CURRENT_BINARY_DIR}/plugingui/locale/da.mo"
			"${CMAKE_CURRENT_BINARY_DIR}/plugingui/locale/fr.mo"
		)
		add_custom_command(
			OUTPUT ${DG_NLS_RESOURCES}
			COMMAND ${MSGFMT_EXECUTABLE} -c -v -o "${CMAKE_CURRENT_BINARY_DIR}/plugingui/locale/da.mo" "${CMAKE_CURRENT_SOURCE_DIR}/plugingui/locale/da.po"
			COMMAND ${MSGFMT_EXECUTABLE} -c -v -o "${CMAKE_CURRENT_BINARY_DIR}/plugingui/locale/fr.mo" "${CMAKE_CURRENT_SOURCE_DIR}/plugingui/locale/fr.po"
			DEPENDS
				"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/locale/da.po"
				"${CMAKE_CURRENT_SOURCE_DIR}/plugingui/locale/fr.po"
			VERBATIM
		)
	endif()

	set(DG_PLUGINGUI_RESOURCE_DATA "${CMAKE_CURRENT_BINARY_DIR}/plugingui/resource_data.cc")
	add_custom_command(
		OUTPUT "${DG_PLUGINGUI_RESOURCE_DATA}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/plugingui"
		COMMAND $<TARGET_FILE:rcgen>
			-s "${CMAKE_CURRENT_SOURCE_DIR}/plugingui/"
			-s "${CMAKE_CURRENT_BINARY_DIR}/plugingui/"
			-o "${DG_PLUGINGUI_RESOURCE_DATA}"
			${DG_PLUGINGUI_RESOURCES}
			${DG_NLS_RESOURCES}
		DEPENDS rcgen ${DG_PLUGINGUI_RESOURCES} ${DG_NLS_RESOURCES}
		VERBATIM
	)
	add_custom_target(dg_plugingui_resource_data
		DEPENDS "${DG_PLUGINGUI_RESOURCE_DATA}"
	)

	add_library(dg_plugingui STATIC
		plugingui/abouttab.cc
		plugingui/bleedcontrolframecontent.cc
		plugingui/diskstreamingframecontent.cc
		plugingui/drumkitframecontent.cc
		plugingui/drumkittab.cc
		plugingui/filebrowser.cc
		plugingui/humaniservisualiser.cc
		plugingui/humanizerframecontent.cc
		plugingui/maintab.cc
		plugingui/mainwindow.cc
		plugingui/pluginconfig.cc
		plugingui/powerwidget.cc
		plugingui/resamplingframecontent.cc
		plugingui/sampleselectionframecontent.cc
		plugingui/statusframecontent.cc
		plugingui/timingframecontent.cc
		plugingui/visualizerframecontent.cc
		plugingui/voicelimitframecontent.cc
		"${DG_PLUGINGUI_RESOURCE_DATA}"
	)
	target_compile_definitions(dg_plugingui PRIVATE
		${DG_COMMON_COMPILE_DEFINITIONS}
		${DG_GUI_COMPILE_DEFINITIONS}
		WITH_HUG_MUTEX
	)
	target_compile_options(dg_plugingui PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
	target_include_directories(dg_plugingui PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${DG_HUGIN_INCLUDE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${CMAKE_CURRENT_BINARY_DIR}"
	)
	target_link_libraries(dg_plugingui PUBLIC dg_dggui dg_core dg_nls Threads::Threads)

	if(DG_ENABLE_PLUGINGUI_TEST)
		add_executable(plugingui
			plugingui/testmain.cc
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		)
		target_compile_definitions(plugingui PRIVATE
			${DG_COMMON_COMPILE_DEFINITIONS}
			${DG_GUI_COMPILE_DEFINITIONS}
			WITH_HUG_MUTEX
		)
		target_compile_options(plugingui PRIVATE ${DG_COMMON_COMPILE_OPTIONS})
		target_include_directories(plugingui PRIVATE
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${DG_HUGIN_INCLUDE_DIR}"
			"${CMAKE_CURRENT_BINARY_DIR}"
		)
		target_link_libraries(plugingui PRIVATE dg_plugingui dg_core dg_nls)
	endif()

	if(DG_ENABLE_LV2)
		add_executable(ttlgen
			"${DG_PLUGINGIZMO_SOURCE_DIR}/ttlgen.cc"
		)
		target_include_directories(ttlgen PRIVATE "${DG_PLUGINGIZMO_INCLUDE_DIR}")
		target_link_libraries(ttlgen PRIVATE ${DG_DL_LIBRARIES})

		set(DG_LV2_UI_TYPE X11UI)
		if(DG_SELECTED_GUI_BACKEND STREQUAL "win32" OR DG_SELECTED_GUI_BACKEND STREQUAL "pugl-win32")
			set(DG_LV2_UI_TYPE WindowsUI)
		elseif(DG_SELECTED_GUI_BACKEND STREQUAL "cocoa" OR DG_SELECTED_GUI_BACKEND STREQUAL "pugl-cocoa")
			set(DG_LV2_UI_TYPE CocoaUI)
		endif()

		add_library(drumgizmo_lv2 MODULE
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/midievent.cc"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/pluginlv2.cc"
			plugin/drumgizmo_plugin.cc
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		)
		set_target_properties(drumgizmo_lv2 PROPERTIES
			PREFIX ""
			OUTPUT_NAME drumgizmo
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin/drumgizmo.lv2"
		)
		target_compile_definitions(drumgizmo_lv2 PRIVATE
			${DG_COMMON_COMPILE_DEFINITIONS}
			${DG_GUI_COMPILE_DEFINITIONS}
			LV2
			LV2_PLUGIN_URI=\"${DG_LV2_PLUGIN_URI}\"
		)
		target_compile_options(drumgizmo_lv2 PRIVATE ${DG_COMMON_COMPILE_OPTIONS} -fvisibility=hidden -Wno-unused-parameter)
		target_include_directories(drumgizmo_lv2 PRIVATE
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${DG_PLUGINGIZMO_INCLUDE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/plugingui"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${DG_HUGIN_INCLUDE_DIR}"
			"${DG_ZITA_RESAMPLER_LIB_DIR}"
			"${CMAKE_CURRENT_BINARY_DIR}"
		)
		target_link_libraries(drumgizmo_lv2 PRIVATE PkgConfig::LV2 dg_plugingui dg_core dg_nls)

		set(DG_LV2_MANIFEST "${CMAKE_BINARY_DIR}/plugin/drumgizmo.lv2/manifest.ttl")
		add_custom_command(
			OUTPUT "${DG_LV2_MANIFEST}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugin/drumgizmo.lv2"
			COMMAND $<TARGET_FILE:ttlgen> "$<TARGET_FILE:drumgizmo_lv2>" "${DG_LV2_MANIFEST}" "${DG_LV2_UI_TYPE}"
			DEPENDS ttlgen drumgizmo_lv2
			VERBATIM
		)
		add_custom_target(drumgizmo_lv2_manifest ALL DEPENDS "${DG_LV2_MANIFEST}")

		install(TARGETS drumgizmo_lv2
			LIBRARY DESTINATION "${DG_LV2_INSTALL_DIR}/drumgizmo.lv2"
			RUNTIME DESTINATION "${DG_LV2_INSTALL_DIR}/drumgizmo.lv2"
		)
		install(FILES "${DG_LV2_MANIFEST}" DESTINATION "${DG_LV2_INSTALL_DIR}/drumgizmo.lv2")
	endif()

	if(DG_ENABLE_VST)
		set(DG_VST_COMPILE_OPTIONS "-D__int64=long long int")
		if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME MATCHES ".*BSD")
			list(APPEND DG_VST_COMPILE_OPTIONS -D__cdecl=)
		endif()

		add_library(dg_vstsdk STATIC
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffectx.cpp"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/audioeffect.cpp"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x/vstplugmain.cpp"
		)
		target_compile_options(dg_vstsdk PRIVATE -w ${DG_VST_COMPILE_OPTIONS})
		target_include_directories(dg_vstsdk PRIVATE
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x"
			"${DG_VST_SDK_PATH}/pluginterfaces/vst2.x"
		)

		add_library(drumgizmo_vst MODULE
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/midievent.cc"
			"${DG_PLUGINGIZMO_SOURCE_DIR}/pluginvst.cc"
			plugin/drumgizmo_plugin.cc
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		)
		set_target_properties(drumgizmo_vst PROPERTIES
			PREFIX ""
			OUTPUT_NAME drumgizmo_vst
		)
		target_compile_definitions(drumgizmo_vst PRIVATE
			${DG_COMMON_COMPILE_DEFINITIONS}
			${DG_GUI_COMPILE_DEFINITIONS}
			VST
		)
		target_compile_options(drumgizmo_vst PRIVATE ${DG_COMMON_COMPILE_OPTIONS} -fvisibility=hidden ${DG_VST_COMPILE_OPTIONS})
		target_include_directories(drumgizmo_vst PRIVATE
			"${CMAKE_CURRENT_SOURCE_DIR}"
			"${DG_PLUGINGIZMO_INCLUDE_DIR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/plugingui"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${DG_HUGIN_INCLUDE_DIR}"
			"${DG_ZITA_RESAMPLER_LIB_DIR}"
			"${CMAKE_CURRENT_BINARY_DIR}"
			"${DG_VST_SDK_PATH}/public.sdk/source/vst2.x"
			"${DG_VST_SDK_PATH}/pluginterfaces/vst2.x"
		)
		target_link_libraries(drumgizmo_vst PRIVATE dg_vstsdk dg_plugingui dg_core dg_nls)

		install(TARGETS drumgizmo_vst
			LIBRARY DESTINATION "${DG_VST_INSTALL_DIR}"
			RUNTIME DESTINATION "${DG_VST_INSTALL_DIR}"
		)
	endif()
endif()

if(DG_ENABLE_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
