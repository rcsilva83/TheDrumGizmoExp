include_guard(GLOBAL)

set(DG_CPM_DOWNLOAD_VERSION "0.40.2")
set(DG_CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${DG_CPM_DOWNLOAD_VERSION}.cmake")
if(NOT EXISTS "${DG_CPM_DOWNLOAD_LOCATION}")
	file(DOWNLOAD
		"https://github.com/cpm-cmake/CPM.cmake/releases/download/v${DG_CPM_DOWNLOAD_VERSION}/CPM.cmake"
		"${DG_CPM_DOWNLOAD_LOCATION}"
		TLS_VERIFY ON
		STATUS DG_CPM_DOWNLOAD_STATUS
	)
	list(GET DG_CPM_DOWNLOAD_STATUS 0 DG_CPM_DOWNLOAD_STATUS_CODE)
	if(NOT DG_CPM_DOWNLOAD_STATUS_CODE EQUAL 0)
		list(GET DG_CPM_DOWNLOAD_STATUS 1 DG_CPM_DOWNLOAD_STATUS_MESSAGE)
		message(FATAL_ERROR "Failed to download CPM.cmake: ${DG_CPM_DOWNLOAD_STATUS_MESSAGE}")
	endif()
endif()

include("${DG_CPM_DOWNLOAD_LOCATION}")

set(DG_CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/cpm-cache" CACHE PATH "Directory for CPM source cache")
set(CPM_SOURCE_CACHE "${DG_CPM_SOURCE_CACHE}")

function(dg_cpm_add_source_dependency package_name repository tag)
	CPMAddPackage(
		NAME ${package_name}
		GIT_REPOSITORY ${repository}
		GIT_TAG ${tag}
		GIT_SHALLOW TRUE
		DOWNLOAD_ONLY YES
	)

	if(NOT DEFINED ${package_name}_SOURCE_DIR)
		message(FATAL_ERROR "CPM failed to resolve dependency '${package_name}'")
	endif()
endfunction()

dg_cpm_add_source_dependency(hugin git://git.drumgizmo.org/hugin.git 3e2e1589dc50bd944cfd63d58a846921966224af)
set(DG_HUGIN_SOURCE_DIR "${hugin_SOURCE_DIR}")

dg_cpm_add_source_dependency(pugixml git://git.drumgizmo.org/pugixml.git e584ea337ede5b33d3e3f2165352a233b67b7fab)
set(DG_PUGIXML_SOURCE_DIR "${pugixml_SOURCE_DIR}")

dg_cpm_add_source_dependency(zita_resampler git://git.drumgizmo.org/zita-resampler.git 76288efeead86fd5799d9771e50bf3222aaa8e71)
set(DG_ZITA_RESAMPLER_SOURCE_DIR "${zita_resampler_SOURCE_DIR}")

if(DG_ENABLE_CLI OR DG_NEED_PLUGIN_GUI)
	dg_cpm_add_source_dependency(getoptpp git://git.drumgizmo.org/getoptpp.git 9ff20ef857429619267e3f156a4f81ad9e1eb8c1)
	set(DG_GETOPTPP_SOURCE_DIR "${getoptpp_SOURCE_DIR}")
endif()

if(DG_NEED_PLUGIN_GUI OR (DG_ENABLE_CLI AND DG_ENABLE_DGVALIDATOR))
	dg_cpm_add_source_dependency(lodepng git://git.drumgizmo.org/lodepng.git 9426bf43031a3b0c37bc0c19b97448f316f5cf7e)
	set(DG_LODEPNG_SOURCE_DIR "${lodepng_SOURCE_DIR}")

	if(DG_NEED_PLUGIN_GUI AND (DG_GUI_BACKEND STREQUAL "pugl-x11" OR DG_GUI_BACKEND STREQUAL "pugl-win32" OR DG_GUI_BACKEND STREQUAL "pugl-cocoa"))
		dg_cpm_add_source_dependency(pugl git://git.drumgizmo.org/pugl.git c4dcd9609a4e0a968c58830301cfb49e2beffd4b)
		set(DG_PUGL_SOURCE_DIR "${pugl_SOURCE_DIR}")
	endif()
endif()

if(DG_ENABLE_LV2 OR DG_ENABLE_VST)
	dg_cpm_add_source_dependency(plugingizmo git://git.drumgizmo.org/plugingizmo.git a88c76afd8fbfe31b76010bac34c1437b1927245)
	set(DG_PLUGINGIZMO_SOURCE_DIR "${plugingizmo_SOURCE_DIR}")
endif()

if(DG_ENABLE_TESTS)
	dg_cpm_add_source_dependency(uunit git://git.drumgizmo.org/uunit.git 0f371777e02dd068f9675a05a29230221d5d6a7f)
	set(DG_UUNIT_SOURCE_DIR "${uunit_SOURCE_DIR}")
endif()
