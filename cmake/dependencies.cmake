set(DG_CPM_VERSION "0.40.8" CACHE STRING "CPM.cmake version")
set(DG_CPM_URL "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${DG_CPM_VERSION}/CPM.cmake" CACHE STRING "CPM.cmake download URL")
set(DG_CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/cpm-cache" CACHE PATH "Directory used by CPM.cmake to cache dependency sources")
set(DG_CPM_FILE "${CMAKE_BINARY_DIR}/cmake/CPM_${DG_CPM_VERSION}.cmake" CACHE FILEPATH "Downloaded CPM.cmake file")

set(DG_HUGIN_GIT_TAG "3e2e1589dc50bd944cfd63d58a846921966224af" CACHE STRING "hugin commit")
set(DG_PLUGINGIZMO_GIT_TAG "be64ddf9da525cd5c6757464efc966052731ba71" CACHE STRING "plugingizmo commit")
set(DG_LODEPNG_GIT_TAG "a71964ed5fe4f82a32ac7f8201338900f66e855d" CACHE STRING "lodepng commit")
set(DG_GETOPTPP_GIT_TAG "9ff20ef857429619267e3f156a4f81ad9e1eb8c1" CACHE STRING "getoptpp commit")
set(DG_PUGL_GIT_TAG "8b8f97dac9dea213875db4cfc3a0bd2033b2f6a1" CACHE STRING "pugl commit")
set(DG_PUGIXML_VERSION "1.15" CACHE STRING "pugixml version")
set(DG_ZITA_RESAMPLER_GIT_TAG "76288efeead86fd5799d9771e50bf3222aaa8e71" CACHE STRING "zita-resampler commit")
set(DG_UUNIT_GIT_TAG "a0ce0a75828401be43132fdfffa89f5ae903f501" CACHE STRING "uunit commit")

if(NOT DEFINED ENV{CPM_SOURCE_CACHE})
	set(ENV{CPM_SOURCE_CACHE} "${DG_CPM_SOURCE_CACHE}")
endif()

if(NOT EXISTS "${DG_CPM_FILE}")
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cmake")
	message(STATUS "Downloading CPM.cmake from ${DG_CPM_URL}")
	file(DOWNLOAD
		"${DG_CPM_URL}"
		"${DG_CPM_FILE}"
		TLS_VERIFY ON
		STATUS DG_CPM_DOWNLOAD_STATUS
	)
	list(GET DG_CPM_DOWNLOAD_STATUS 0 DG_CPM_DOWNLOAD_CODE)
	list(GET DG_CPM_DOWNLOAD_STATUS 1 DG_CPM_DOWNLOAD_MESSAGE)
	if(NOT DG_CPM_DOWNLOAD_CODE EQUAL 0)
		message(FATAL_ERROR "Failed downloading CPM.cmake: ${DG_CPM_DOWNLOAD_MESSAGE}")
	endif()
endif()

include("${DG_CPM_FILE}")

CPMAddPackage(
	NAME hugin
	GIT_REPOSITORY git://git.drumgizmo.org/hugin.git
	GIT_TAG ${DG_HUGIN_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME pluginizmo
	GIT_REPOSITORY git://git.drumgizmo.org/plugingizmo.git
	GIT_TAG ${DG_PLUGINGIZMO_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME lodepng
	GIT_REPOSITORY git://git.drumgizmo.org/lodepng.git
	GIT_TAG ${DG_LODEPNG_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME getoptpp
	GIT_REPOSITORY git://git.drumgizmo.org/getoptpp.git
	GIT_TAG ${DG_GETOPTPP_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME pugl
	GIT_REPOSITORY git://git.drumgizmo.org/pugl.git
	GIT_TAG ${DG_PUGL_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME pugixml
	GITHUB_REPOSITORY zeux/pugixml
	VERSION ${DG_PUGIXML_VERSION}
	OPTIONS
		"BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
	NAME zitaresampler
	GIT_REPOSITORY git://git.drumgizmo.org/zita-resampler.git
	GIT_TAG ${DG_ZITA_RESAMPLER_GIT_TAG}
	DOWNLOAD_ONLY YES
)

CPMAddPackage(
	NAME uunit
	GIT_REPOSITORY git://git.drumgizmo.org/uunit.git
	GIT_TAG ${DG_UUNIT_GIT_TAG}
	DOWNLOAD_ONLY YES
)

set(DG_HUGIN_SOURCE_DIR "${hugin_SOURCE_DIR}")
set(DG_PLUGINGIZMO_SOURCE_DIR "${pluginizmo_SOURCE_DIR}")
set(DG_LODEPNG_SOURCE_DIR "${lodepng_SOURCE_DIR}")
set(DG_GETOPTPP_SOURCE_DIR "${getoptpp_SOURCE_DIR}")
set(DG_PUGL_SOURCE_DIR "${pugl_SOURCE_DIR}")
set(DG_PUGIXML_SOURCE_DIR "${pugixml_SOURCE_DIR}")
set(DG_ZITA_RESAMPLER_SOURCE_DIR "${zitaresampler_SOURCE_DIR}")
set(DG_UUNIT_SOURCE_DIR "${uunit_SOURCE_DIR}")

if(NOT EXISTS "${DG_HUGIN_SOURCE_DIR}/hugin.h")
	message(FATAL_ERROR "CPM failed to provide hugin sources")
endif()

if(NOT EXISTS "${DG_PLUGINGIZMO_SOURCE_DIR}/pluginlv2.cc")
	message(FATAL_ERROR "CPM failed to provide pluggingizmo sources")
endif()

if(NOT EXISTS "${DG_LODEPNG_SOURCE_DIR}/lodepng.cpp")
	message(FATAL_ERROR "CPM failed to provide lodepng sources")
endif()

if(NOT EXISTS "${DG_GETOPTPP_SOURCE_DIR}/getoptpp.hpp")
	message(FATAL_ERROR "CPM failed to provide getoptpp sources")
endif()

if(NOT EXISTS "${DG_ZITA_RESAMPLER_SOURCE_DIR}/libs/resampler.cc")
	message(FATAL_ERROR "CPM failed to provide zita-resampler sources")
endif()

if(NOT EXISTS "${DG_UUNIT_SOURCE_DIR}/uunit.cc")
	message(FATAL_ERROR "CPM failed to provide uunit sources")
endif()

if(NOT TARGET pugixml)
	message(FATAL_ERROR "CPM failed to provide the pugixml target")
endif()
