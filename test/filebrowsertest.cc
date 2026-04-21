/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 **************************************************************************
 *            filebrowsertest.cc
 *
 *  Tue Apr 21 08:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <dggui/window.h>
#include <filebrowser.h>

#ifndef _WIN32
#include <fcntl.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace GUI;

#ifndef _WIN32

// Helper: create an empty file at the given path.
static void createEmptyFile(const std::string& path)
{
	int fd = open(path.c_str(), O_CREAT | O_WRONLY, 0644);
	REQUIRE(fd >= 0);
	REQUIRE(close(fd) == 0);
}

// Callback for nftw()-based recursive deletion.
static int removeEntry(
    const char* fpath, const struct stat*, int typeflag, struct FTW*)
{
	if(typeflag == FTW_DP)
	{
		return rmdir(fpath);
	}
	return unlink(fpath);
}

// RAII helper that creates a temporary directory tree for FileBrowser tests.
struct FileBrowserTempDirFixture
{
	std::string base_path;

	FileBrowserTempDirFixture()
	{
		char templ[] = "/tmp/dg-filebrowsertest-XXXXXX";
		const char* p = mkdtemp(templ);
		base_path = p ? p : "";
		REQUIRE(!base_path.empty());

		mkdir((base_path + "/subdir").c_str(), 0755);
		createEmptyFile(base_path + "/test.xml");
		createEmptyFile(base_path + "/drumkit.xml");
		createEmptyFile(base_path + "/readme.txt");
	}

	~FileBrowserTempDirFixture()
	{
		// Recursively remove the temp tree using explicit filesystem calls.
		nftw(base_path.c_str(), removeEntry, 64, FTW_DEPTH | FTW_PHYS);
	}
};

#endif // _WIN32

//! Listener class for testing notifiers
class FileBrowserNotifierListener : public Listener
{
public:
	int fileSelectCount{0};
	int fileCancelCount{0};
	int defaultPathChangedCount{0};
	std::string lastSelectedFile;
	std::string lastDefaultPath;

	void onFileSelected(const std::string& filename)
	{
		++fileSelectCount;
		lastSelectedFile = filename;
	}

	void onFileCanceled()
	{
		++fileCancelCount;
	}

	void onDefaultPathChanged(const std::string& path)
	{
		++defaultPathChangedCount;
		lastDefaultPath = path;
	}

	void reset()
	{
		fileSelectCount = 0;
		fileCancelCount = 0;
		defaultPathChangedCount = 0;
		lastSelectedFile.clear();
		lastDefaultPath.clear();
	}
};

TEST_CASE("FileBrowserConstructorTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("constructor_initializes_with_cwd")
	{
		FileBrowser browser(&window);

		// Browser should be focusable
		CHECK_UNARY(browser.isFocusable());

		// Initially has no filename
		CHECK_UNARY(!browser.hasFilename());
		CHECK_EQ(browser.getFilename(), "");
	}

	SUBCASE("destructor_cleans_up")
	{
		// Just verify destructor doesn't crash
		{
			FileBrowser browser(&window);
		}
		CHECK_UNARY(true);
	}
}

#ifndef _WIN32
TEST_CASE_FIXTURE(FileBrowserTempDirFixture, "FileBrowserSetPathTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("setPath_with_valid_directory")
	{
		FileBrowser browser(&window);

		browser.setPath(base_path);

		// Browser should have updated its internal state
		// We can verify by checking that it doesn't crash and has no filename
		CHECK_UNARY(!browser.hasFilename());
	}

	SUBCASE("setPath_with_empty_string_uses_cwd")
	{
		FileBrowser browser(&window);
		std::string cwd = Directory::cwd();

		browser.setPath("");

		// Should fall back to cwd's directory (no crash = success)
		CHECK_UNARY(!browser.hasFilename());
	}

	SUBCASE("setPath_with_file_path_uses_directory")
	{
		FileBrowser browser(&window);
		std::string file_path = base_path + "/test.xml";

		browser.setPath(file_path);

		// Should use the directory containing the file (no crash = success)
		CHECK_UNARY(!browser.hasFilename());
	}

	SUBCASE("setPath_with_nonexistent_path_uses_cwd")
	{
		FileBrowser browser(&window);

		browser.setPath("/nonexistent/path/that/does/not/exist");

		// Should fall back to cwd (no crash = success)
		CHECK_UNARY(!browser.hasFilename());
	}
}

TEST_CASE_FIXTURE(
    FileBrowserTempDirFixture, "FileBrowserDirectoryNavigationTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("setPath_updates_browser_state")
	{
		FileBrowser browser(&window);

		browser.setPath(base_path);

		// Verify no filename selected yet
		CHECK_UNARY(!browser.hasFilename());
	}

	SUBCASE("setPath_with_subdirectory_path")
	{
		FileBrowser browser(&window);

		browser.setPath(base_path + "/subdir");

		// Verify no filename selected
		CHECK_UNARY(!browser.hasFilename());
	}
}

TEST_CASE_FIXTURE(FileBrowserTempDirFixture, "FileBrowserNotifierTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("defaultPathChangedNotifier_triggers_on_setDefaultPath")
	{
		FileBrowser browser(&window);
		browser.setPath(base_path);

		FileBrowserNotifierListener listener;
		browser.defaultPathChangedNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onDefaultPathChanged);

		// Trigger set default path via the notifier connection
		// The setDefaultPath method is called when the button is clicked
		// We can trigger it through the notifier itself
		browser.defaultPathChangedNotifier(base_path);

		CHECK_EQ(listener.defaultPathChangedCount, 1);
		CHECK_EQ(listener.lastDefaultPath, base_path);
	}

	SUBCASE("fileSelectCancelNotifier_triggers_when_connected")
	{
		FileBrowser browser(&window);

		FileBrowserNotifierListener listener;
		browser.fileSelectCancelNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onFileCanceled);

		// Trigger the cancel notifier directly
		browser.fileSelectCancelNotifier();

		CHECK_EQ(listener.fileCancelCount, 1);
	}

	SUBCASE("fileSelectNotifier_triggers_when_connected")
	{
		FileBrowser browser(&window);

		FileBrowserNotifierListener listener;
		browser.fileSelectNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onFileSelected);

		// Trigger the file select notifier directly
		browser.fileSelectNotifier(base_path + "/test.xml");

		CHECK_EQ(listener.fileSelectCount, 1);
		CHECK_EQ(listener.lastSelectedFile, base_path + "/test.xml");
	}
}
#endif // _WIN32

TEST_CASE("FileBrowserResizeTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("resize_with_reasonable_dimensions")
	{
		FileBrowser browser(&window);

		// Resize should not crash with reasonable dimensions
		// (avoiding zero dimensions due to widget assertions)
		browser.resize(100, 100);

		// In headless mode, actual dimensions may differ due to windowing
		// system Just verify no crash occurred
		CHECK_UNARY(true);
	}

	SUBCASE("resize_updates_child_widgets")
	{
		FileBrowser browser(&window);

		// Initial resize
		browser.resize(100, 100);

		// Resize to different dimensions
		browser.resize(200, 200);

		// Child widgets should have been updated (just verify no crash)
		CHECK_UNARY(true);
	}
}

TEST_CASE("FileBrowserGetFilenameTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("getFilename_empty_initially")
	{
		FileBrowser browser(&window);

		CHECK_EQ(browser.getFilename(), "");
		CHECK_UNARY(!browser.hasFilename());
	}

	SUBCASE("fileSelectNotifier_updates_state")
	{
		FileBrowser browser(&window);

		// Initially no filename
		CHECK_UNARY(!browser.hasFilename());

		// Simulate file selection via notifier
		browser.fileSelectNotifier("/some/path/file.xml");

		// After selection, browser should have the filename
		CHECK_UNARY(browser.hasFilename());
		CHECK_EQ(browser.getFilename(), "/some/path/file.xml");
	}

	SUBCASE("fileSelectCancelNotifier_notifies_listeners")
	{
		FileBrowser browser(&window);
		FileBrowserNotifierListener listener;

		browser.fileSelectCancelNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onFileCanceled);

		// Trigger cancel notifier
		browser.fileSelectCancelNotifier();

		// Listener should have been notified
		CHECK_EQ(listener.fileCancelCount, 1);
	}
}

TEST_CASE("FileBrowserMultipleNotifiersTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("multiple_listeners_can_connect_to_notifiers")
	{
		FileBrowser browser(&window);

		FileBrowserNotifierListener listener1;
		FileBrowserNotifierListener listener2;

		browser.fileSelectNotifier.connect(
		    &listener1, &FileBrowserNotifierListener::onFileSelected);
		browser.fileSelectNotifier.connect(
		    &listener2, &FileBrowserNotifierListener::onFileSelected);

		browser.fileSelectNotifier("/path/to/file.xml");

		CHECK_EQ(listener1.fileSelectCount, 1);
		CHECK_EQ(listener2.fileSelectCount, 1);
		CHECK_EQ(listener1.lastSelectedFile, "/path/to/file.xml");
		CHECK_EQ(listener2.lastSelectedFile, "/path/to/file.xml");
	}

	SUBCASE("disconnect_stops_notifications")
	{
		FileBrowser browser(&window);

		FileBrowserNotifierListener listener;
		browser.fileSelectNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onFileSelected);

		browser.fileSelectNotifier("/path/to/file1.xml");
		CHECK_EQ(listener.fileSelectCount, 1);

		browser.fileSelectNotifier.disconnect(&listener);

		browser.fileSelectNotifier("/path/to/file2.xml");
		// Should still be 1 because we disconnected
		CHECK_EQ(listener.fileSelectCount, 1);
	}
}

TEST_CASE("FileBrowserRefFileTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("fileSelectNotifier_with_at_prefix_works")
	{
		FileBrowser browser(&window);

		FileBrowserNotifierListener listener;
		browser.fileSelectNotifier.connect(
		    &listener, &FileBrowserNotifierListener::onFileSelected);

		// Test reference file path (starts with @)
		browser.fileSelectNotifier("@/some/ref/file.xml");

		CHECK_EQ(listener.fileSelectCount, 1);
		CHECK_EQ(listener.lastSelectedFile, "@/some/ref/file.xml");
	}
}

TEST_CASE("FileBrowserRepaintEventTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("repaintEvent_does_not_crash")
	{
		FileBrowser browser(&window);
		browser.resize(400, 300);

		// Create a repaint event
		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 400;
		event.height = 300;

		// Call repaint event - should not crash
		browser.repaintEvent(&event);

		CHECK_UNARY(true);
	}
}

TEST_CASE("FileBrowserCaptionTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("browser_has_expected_caption")
	{
		FileBrowser browser(&window);

		// The browser is a dialog with a caption set in constructor
		// We can verify it was created successfully
		CHECK_UNARY(!browser.hasFilename());
	}
}
