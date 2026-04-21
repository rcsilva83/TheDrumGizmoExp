include(CMakeParseArguments)

set(DG_TEST_COMMON_INCLUDE_DIRS
	"${DG_DOCTEST_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_BINARY_DIR}"
)

function(dg_add_doctest_test target)
	set(options)
	set(one_value_args)
	set(multi_value_args SOURCES INCLUDE_DIRS LINK_LIBRARIES COMPILE_DEFINITIONS COMPILE_OPTIONS DEPENDENCIES)
	cmake_parse_arguments(DG_TEST "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

	add_executable(${target}
		"${CMAKE_SOURCE_DIR}/test/doctest_main.cc"
		${DG_TEST_SOURCES}
	)

	target_compile_definitions(${target} PRIVATE
		OUTPUT=\"${target}\"
		${DG_TEST_COMPILE_DEFINITIONS}
	)
	if(DG_COMMON_COMPILE_OPTIONS OR DG_TEST_COMPILE_OPTIONS)
		target_compile_options(${target} PRIVATE
			${DG_COMMON_COMPILE_OPTIONS}
			${DG_TEST_COMPILE_OPTIONS}
		)
	endif()
	target_include_directories(${target} PRIVATE
		${DG_TEST_COMMON_INCLUDE_DIRS}
		${DG_TEST_INCLUDE_DIRS}
	)
	if(DG_TEST_LINK_LIBRARIES)
		target_link_libraries(${target} PRIVATE ${DG_TEST_LINK_LIBRARIES})
	endif()
	if(DG_TEST_DEPENDENCIES)
		add_dependencies(${target} ${DG_TEST_DEPENDENCIES})
	endif()

	add_test(NAME ${target} COMMAND ${target})
endfunction()

if(TARGET dg_plugingui_resource_data AND DG_PLUGINGUI_RESOURCE_DATA)
	set_source_files_properties("${DG_PLUGINGUI_RESOURCE_DATA}" PROPERTIES GENERATED TRUE)

	dg_add_doctest_test(resource
		SOURCES
			"${CMAKE_SOURCE_DIR}/dggui/resource.cc"
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/src/random.cc"
			"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
			"${CMAKE_SOURCE_DIR}/test/resource_test.cc"
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		DEPENDENCIES
			dg_plugingui_resource_data
		LINK_LIBRARIES
			SndFile::sndfile
	)
endif()

dg_add_doctest_test(audiocache
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiocache.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocacheeventhandler.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocachefile.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocacheidmanager.cc"
		"${CMAKE_SOURCE_DIR}/src/thread.cc"
		"${CMAKE_SOURCE_DIR}/src/sem.cc"
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/audiocachetest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
		DG_ENABLE_TESTS
	LINK_LIBRARIES
		Threads::Threads
		SndFile::sndfile
)

dg_add_doctest_test(audiocachefile
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiocachefile.cc"
		"${CMAKE_SOURCE_DIR}/src/thread.cc"
		"${CMAKE_SOURCE_DIR}/src/sem.cc"
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/audiocachefiletest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		Threads::Threads
		SndFile::sndfile
)

dg_add_doctest_test(audiocacheidmanager
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiocacheidmanager.cc"
		"${CMAKE_SOURCE_DIR}/test/audiocacheidmanagertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		SndFile::sndfile
)

dg_add_doctest_test(audiocacheeventhandler
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiocacheeventhandler.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocacheidmanager.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocachefile.cc"
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/src/thread.cc"
		"${CMAKE_SOURCE_DIR}/src/sem.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/audiocacheeventhandlertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		Threads::Threads
		SndFile::sndfile
)

dg_add_doctest_test(framesizetest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiocache.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocacheeventhandler.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocachefile.cc"
		"${CMAKE_SOURCE_DIR}/src/audiocacheidmanager.cc"
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/thread.cc"
		"${CMAKE_SOURCE_DIR}/src/sem.cc"
		"${CMAKE_SOURCE_DIR}/test/framesizetest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		Threads::Threads
		SndFile::sndfile
)

dg_add_doctest_test(enginetest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/enginetest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_core
		Threads::Threads
		SndFile::sndfile
)

if(TARGET dg_dggui AND TARGET dg_plugingui_resource_data)
	dg_add_doctest_test(paintertest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/paintertest.cc"
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		DEPENDENCIES
			dg_plugingui_resource_data
		LINK_LIBRARIES
			dg_dggui
			Threads::Threads
	)
endif()

dg_add_doctest_test(configfile
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/configfile.cc"
		"${CMAKE_SOURCE_DIR}/src/directory.cc"
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/test/configtest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
)

dg_add_doctest_test(randomtest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/test/randomtest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(sampleselectiontest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/powerlist.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/src/sample.cc"
		"${CMAKE_SOURCE_DIR}/src/sample_selection.cc"
		"${CMAKE_SOURCE_DIR}/test/sampleselectiontest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		SndFile::sndfile
)

dg_add_doctest_test(atomictest
	SOURCES
		"${CMAKE_SOURCE_DIR}/test/atomictest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
)

dg_add_doctest_test(syncedsettingstest
	SOURCES
		"${CMAKE_SOURCE_DIR}/test/syncedsettings.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
)

if(TARGET dg_dggui AND TARGET dg_plugingui)
	dg_add_doctest_test(imagecachetest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/imagecachetest.cc"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_dggui
			dg_plugingui
	)
endif()

dg_add_doctest_test(semaphoretest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/sem.cc"
		"${CMAKE_SOURCE_DIR}/test/semaphoretest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		Threads::Threads
)

dg_add_doctest_test(drumkitcreatortest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkitcreatortest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		SndFile::sndfile
)

dg_add_doctest_test(bytesizeparsertest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/bytesizeparser.cc"
		"${CMAKE_SOURCE_DIR}/test/bytesizeparsertest.cc"
)

dg_add_doctest_test(notifiertest
	SOURCES
		"${CMAKE_SOURCE_DIR}/test/notifiertest.cc"
)

dg_add_doctest_test(dgxmlparsertest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/dgxmlparser.cc"
		"${CMAKE_SOURCE_DIR}/test/dgxmlparsertest.cc"
		"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_pugi
)

dg_add_doctest_test(domloadertest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/test/domloadertest.cc"
		"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_core
)

dg_add_doctest_test(configparsertest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/configparser.cc"
		"${CMAKE_SOURCE_DIR}/test/configparsertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_pugi
)

dg_add_doctest_test(midimapparsertest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/midimapparser.cc"
		"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
		"${CMAKE_SOURCE_DIR}/test/midimapparsertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_pugi
)

dg_add_doctest_test(eventsdstest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/events_ds.cc"
		"${CMAKE_SOURCE_DIR}/test/eventsdstest.cc"
	LINK_LIBRARIES
		SndFile::sndfile
)

dg_add_doctest_test(powermaptest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/powermap.cc"
		"${CMAKE_SOURCE_DIR}/test/powermaptest.cc"
)

dg_add_doctest_test(channelmixertest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/channel.cc"
		"${CMAKE_SOURCE_DIR}/src/channelmixer.cc"
		"${CMAKE_SOURCE_DIR}/test/channelmixertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		SndFile::sndfile
)

dg_add_doctest_test(audiofiletest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
		"${CMAKE_SOURCE_DIR}/test/audiofiletest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		Threads::Threads
		SndFile::sndfile
)

dg_add_doctest_test(rangemaptest
	SOURCES
		"${CMAKE_SOURCE_DIR}/test/rangemaptest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(inputfilterstest
	SOURCES
		"${CMAKE_SOURCE_DIR}/src/latencyfilter.cc"
		"${CMAKE_SOURCE_DIR}/src/powermap.cc"
		"${CMAKE_SOURCE_DIR}/src/powermapfilter.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/src/staminafilter.cc"
		"${CMAKE_SOURCE_DIR}/src/velocityfilter.cc"
		"${CMAKE_SOURCE_DIR}/test/inputfilterstest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(versionstrtest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/versionstr.cc"
		"${CMAKE_SOURCE_DIR}/test/versionstrtest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(directorytest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/directory.cc"
		"${CMAKE_SOURCE_DIR}/test/directorytest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(midimappertest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/midimapper.cc"
		"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
		"${CMAKE_SOURCE_DIR}/test/midimappertest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	LINK_LIBRARIES
		dg_pugi
)

dg_add_doctest_test(audioinputenginemiditest
	SOURCES
		"${DG_HUGIN_SOURCE_DIR}/hugin.c"
		"${CMAKE_SOURCE_DIR}/src/audioinputenginemidi.cc"
		"${CMAKE_SOURCE_DIR}/src/audiofile.cc"
		"${CMAKE_SOURCE_DIR}/src/configfile.cc"
		"${CMAKE_SOURCE_DIR}/src/directory.cc"
		"${CMAKE_SOURCE_DIR}/src/instrument.cc"
		"${CMAKE_SOURCE_DIR}/src/midimapper.cc"
		"${CMAKE_SOURCE_DIR}/src/midimapparser.cc"
		"${CMAKE_SOURCE_DIR}/src/powerlist.cc"
		"${CMAKE_SOURCE_DIR}/src/random.cc"
		"${CMAKE_SOURCE_DIR}/src/sample.cc"
		"${CMAKE_SOURCE_DIR}/src/sample_selection.cc"
		"${CMAKE_SOURCE_DIR}/src/versionstr.cc"
		"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
		"${CMAKE_SOURCE_DIR}/test/audioinputenginemiditest.cc"
	INCLUDE_DIRS
		"${DG_HUGIN_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
	LINK_LIBRARIES
		dg_pugi
		SndFile::sndfile
)

dg_add_doctest_test(colourtest
	SOURCES
		"${CMAKE_SOURCE_DIR}/dggui/colour.cc"
		"${CMAKE_SOURCE_DIR}/test/colourtest.cc"
	INCLUDE_DIRS
		"${CMAKE_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(utf8test
	SOURCES
		"${CMAKE_SOURCE_DIR}/dggui/utf8.cc"
		"${CMAKE_SOURCE_DIR}/test/utf8test.cc"
	INCLUDE_DIRS
		"${CMAKE_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

dg_add_doctest_test(pixelbuffertest
	SOURCES
		"${CMAKE_SOURCE_DIR}/dggui/pixelbuffer.cc"
		"${CMAKE_SOURCE_DIR}/dggui/colour.cc"
		"${CMAKE_SOURCE_DIR}/test/pixelbuffertest.cc"
	INCLUDE_DIRS
		"${CMAKE_SOURCE_DIR}"
	COMPILE_DEFINITIONS
		DISABLE_HUGIN
)

if(TARGET dg_dggui)
	dg_add_doctest_test(layouttest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/layouttest.cc"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_dggui
			Threads::Threads
	)

	dg_add_doctest_test(imagetest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/dggui/image.cc"
			"${CMAKE_SOURCE_DIR}/dggui/resource.cc"
			"${CMAKE_SOURCE_DIR}/dggui/colour.cc"
			"${CMAKE_SOURCE_DIR}/src/random.cc"
			"${CMAKE_SOURCE_DIR}/test/imagetest.cc"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		COMPILE_DEFINITIONS
			TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}/test"
		LINK_LIBRARIES
			dg_dggui
			Threads::Threads
			SndFile::sndfile
	)

	dg_add_doctest_test(widgettest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/widgettest.cc"
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_dggui
			Threads::Threads
		DEPENDENCIES
			dg_plugingui_resource_data
	)

	dg_add_doctest_test(eventhandler
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/eventhandlertest.cc"
			"${DG_PLUGINGUI_RESOURCE_DATA}"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_dggui
			Threads::Threads
		DEPENDENCIES
			dg_plugingui_resource_data
	)

	# NativeWindowX11 tests - only when X11 backend is actually selected and available
	if(DG_SELECTED_GUI_BACKEND STREQUAL "x11" AND TARGET PkgConfig::X11 AND TARGET PkgConfig::XEXT)
		dg_add_doctest_test(nativewindow_x11
			SOURCES
				"${DG_HUGIN_SOURCE_DIR}/hugin.c"
				"${CMAKE_SOURCE_DIR}/test/nativewindow_x11_test.cc"
			INCLUDE_DIRS
				"${CMAKE_SOURCE_DIR}"
				"${DG_HUGIN_SOURCE_DIR}"
			LINK_LIBRARIES
				dg_dggui
				Threads::Threads
				PkgConfig::X11
				PkgConfig::XEXT
		)
	endif()
endif()

if(TARGET dg_plugingui)
	dg_add_doctest_test(pluginconfigtest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/pluginconfigtest.cc"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_plugingui
			dg_core
			Threads::Threads
	)

	if(DG_PLUGINGUI_RESOURCE_DATA)
		set_source_files_properties("${DG_PLUGINGUI_RESOURCE_DATA}" PROPERTIES GENERATED TRUE)
		dg_add_doctest_test(pluginguitest
			SOURCES
				"${DG_HUGIN_SOURCE_DIR}/hugin.c"
				"${CMAKE_SOURCE_DIR}/test/pluginguitest.cc"
				"${DG_PLUGINGUI_RESOURCE_DATA}"
			INCLUDE_DIRS
				"${CMAKE_SOURCE_DIR}"
				"${DG_HUGIN_SOURCE_DIR}"
			LINK_LIBRARIES
				dg_plugingui
				dg_core
				Threads::Threads
			DEPENDENCIES
				dg_plugingui_resource_data
		)
	endif()

	dg_add_doctest_test(drumkittabtest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/test/drumkittabtest.cc"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		LINK_LIBRARIES
			dg_plugingui
			dg_core
			Threads::Threads
	)
endif()

if(DG_ENABLE_CLI)
	dg_add_doctest_test(drumgizmoclitest
		SOURCES
			"${CMAKE_SOURCE_DIR}/src/random.cc"
			"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
			"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
			"${CMAKE_SOURCE_DIR}/test/drumgizmoclitest.cc"
		INCLUDE_DIRS
			"${DG_HUGIN_SOURCE_DIR}"
		COMPILE_DEFINITIONS
			DRUMGIZMO_CLI_BIN="$<TARGET_FILE:drumgizmo>"
		LINK_LIBRARIES
			SndFile::sndfile
		DEPENDENCIES
			drumgizmo
	)
endif()

if(DG_ENABLE_CLI AND DG_ENABLE_DGVALIDATOR)
	dg_add_doctest_test(dgvalidatortest
		SOURCES
			"${CMAKE_SOURCE_DIR}/src/random.cc"
			"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
			"${CMAKE_SOURCE_DIR}/test/scopedfile.cc"
			"${CMAKE_SOURCE_DIR}/test/dgvalidatortest.cc"
		INCLUDE_DIRS
			"${DG_HUGIN_SOURCE_DIR}"
		COMPILE_DEFINITIONS
			DGVALIDATOR_BIN="$<TARGET_FILE:dgvalidator>"
		LINK_LIBRARIES
			SndFile::sndfile
		DEPENDENCIES
			dgvalidator
	)
endif()

if(DG_ENABLE_LV2)
	dg_add_doctest_test(lv2test
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/src/random.cc"
			"${CMAKE_SOURCE_DIR}/test/drumkit_creator.cc"
			"${CMAKE_SOURCE_DIR}/test/lv2_test_host.cc"
			"${CMAKE_SOURCE_DIR}/test/lv2.cc"
		INCLUDE_DIRS
			"${DG_HUGIN_SOURCE_DIR}"
		COMPILE_DEFINITIONS
			LV2_PATH=\"${CMAKE_BINARY_DIR}/plugin\"
		LINK_LIBRARIES
			PkgConfig::LV2
			PkgConfig::LILV
			PkgConfig::SERD
			OpenSSL::Crypto
			SndFile::sndfile
			${DG_DL_LIBRARIES}
		DEPENDENCIES
			drumgizmo_lv2
			drumgizmo_lv2_manifest
	)
endif()

if(DG_WITH_NLS AND TARGET rcgen)
	set(DG_TRANSLATIONTEST_RESOURCE_DATA "${CMAKE_BINARY_DIR}/test/translationtest_resource_data.cc")
	add_custom_command(
		OUTPUT "${DG_TRANSLATIONTEST_RESOURCE_DATA}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/test"
		COMMAND $<TARGET_FILE:rcgen>
			-s "${CMAKE_SOURCE_DIR}/test/"
			-o "${DG_TRANSLATIONTEST_RESOURCE_DATA}"
			"${CMAKE_SOURCE_DIR}/test/locale/da.mo"
		DEPENDS
			rcgen
			"${CMAKE_SOURCE_DIR}/test/locale/da.mo"
		VERBATIM
	)

	dg_add_doctest_test(translationtest
		SOURCES
			"${DG_HUGIN_SOURCE_DIR}/hugin.c"
			"${CMAKE_SOURCE_DIR}/dggui/resource.cc"
			"${CMAKE_SOURCE_DIR}/src/translation.cc"
			"${CMAKE_SOURCE_DIR}/dggui/uitranslation.cc"
			"${CMAKE_SOURCE_DIR}/test/translationtest.cc"
			"${DG_TRANSLATIONTEST_RESOURCE_DATA}"
		INCLUDE_DIRS
			"${CMAKE_SOURCE_DIR}"
			"${DG_HUGIN_SOURCE_DIR}"
		COMPILE_DEFINITIONS
			MO_SRC=\"${CMAKE_SOURCE_DIR}/test/locale/da.mo\"
		COMPILE_OPTIONS
			-Wno-invalid-source-encoding
	)
endif()
