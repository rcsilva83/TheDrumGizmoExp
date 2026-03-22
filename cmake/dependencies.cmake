set(DG_CPM_VERSION "0.40.8" CACHE STRING "CPM.cmake version")
set(DG_CPM_URL "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${DG_CPM_VERSION}/CPM.cmake" CACHE STRING "CPM.cmake download URL")
set(DG_CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/cpm-cache" CACHE PATH "Directory used by CPM.cmake to cache dependency sources")
set(DG_CPM_FILE "${CMAKE_BINARY_DIR}/cmake/CPM_${DG_CPM_VERSION}.cmake" CACHE FILEPATH "Downloaded CPM.cmake file")

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
