/* -*- Mode: c++ -*- */
/***************************************************************************
 *            directorytest.cc
 *
 *  Mon Mar 23 20:00:00 CET 2026
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

#include <algorithm>

#ifndef _WIN32
#include <fcntl.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "../src/directory.h"

#ifndef _WIN32

// Helper: create an empty file at the given path.
static void create_empty_file(const std::string& path)
{
	int fd = open(path.c_str(), O_CREAT | O_WRONLY, 0644);
	REQUIRE(fd >= 0);
	REQUIRE(close(fd) == 0);
}

// Callback for nftw()-based recursive deletion.
static int remove_entry(const char* fpath, const struct stat*, int typeflag,
    struct FTW*)
{
	if(typeflag == FTW_DP)
	{
		return rmdir(fpath);
	}
	return unlink(fpath);
}

// RAII helper that creates a temporary directory tree for Directory tests.
// Layout under base_path/:
//   alpha/          – subdirectory (comes before beta/ alphabetically)
//   beta/           – subdirectory
//   .hidden/        – hidden subdirectory (filtered by DIRECTORY_HIDDEN)
//   drum.xml        – XML file (should appear in entryList)
//   kit.xml         – XML file (should appear in entryList)
//   readme.txt      – non-XML file (filtered out)
//   x               – name shorter than ".xml" suffix length (filtered out)
struct TempDirFixture
{
	std::string base_path;

	TempDirFixture()
	{
		char templ[] = "/tmp/dg-dirtest-XXXXXX";
		const char* p = mkdtemp(templ);
		base_path = p ? p : "";
		REQUIRE(!base_path.empty());

		mkdir((base_path + "/alpha").c_str(), 0755);
		mkdir((base_path + "/beta").c_str(), 0755);
		mkdir((base_path + "/.hidden").c_str(), 0755);
		create_empty_file(base_path + "/drum.xml");
		create_empty_file(base_path + "/kit.xml");
		create_empty_file(base_path + "/readme.txt");
		create_empty_file(base_path + "/x");
	}

	~TempDirFixture()
	{
		// Recursively remove the temp tree using explicit filesystem calls.
		nftw(base_path.c_str(), remove_entry, 64, FTW_DEPTH | FTW_PHYS);
	}
};

#endif // _WIN32

TEST_CASE("DirectoryTest")
{
	SUBCASE("cleanPath_removes_double_slashes")
	{
		CHECK_EQ(Directory::cleanPath("/foo//bar"), "/foo/bar");
	}

	SUBCASE("cleanPath_resolves_dotdot")
	{
		CHECK_EQ(Directory::cleanPath("/foo/bar/../baz"), "/foo/baz");
	}

	SUBCASE("cleanPath_resolves_multiple_dotdot")
	{
		CHECK_EQ(Directory::cleanPath("/a/b/c/../../d"), "/a/d");
	}

	SUBCASE("cleanPath_root_stays_root")
	{
		CHECK_EQ(Directory::cleanPath("/"), "/");
	}

	SUBCASE("cleanPath_trailing_slash_ignored")
	{
		// A trailing slash is treated like "." so it should not add an extra
		// path component.
		std::string result = Directory::cleanPath("/foo/bar/");
		CHECK_EQ(result, "/foo/bar");
	}

	SUBCASE("isRoot_true_for_slash")
	{
		CHECK_UNARY(Directory::isRoot("/"));
	}

	SUBCASE("isRoot_false_for_non_root")
	{
		CHECK_UNARY(!Directory::isRoot("/foo"));
		CHECK_UNARY(!Directory::isRoot("/foo/bar"));
	}

	SUBCASE("isHidden_true_for_dot_prefix")
	{
		// On POSIX, an entry whose name starts with a single dot (but is not
		// just "..") is considered hidden.
		CHECK_UNARY(Directory::isHidden("/some/path/.hidden"));
		CHECK_UNARY(Directory::isHidden("/home/user/.config"));
	}

	SUBCASE("isHidden_false_for_normal_entry")
	{
		CHECK_UNARY(!Directory::isHidden("/some/path/visible"));
		CHECK_UNARY(!Directory::isHidden("/some/path/file.txt"));
	}

	SUBCASE("isHidden_false_for_dotdot")
	{
		// ".." (parent directory) must NOT be treated as hidden.
		CHECK_UNARY(!Directory::isHidden("/some/path/.."));
	}

	SUBCASE("isDir_false_for_nonexistent_path")
	{
		CHECK_UNARY(!Directory::isDir("/this/path/does/not/exist"));
	}

	SUBCASE("isDir_true_for_real_directory")
	{
		CHECK_UNARY(Directory::isDir("/tmp"));
	}

	SUBCASE("exists_false_for_nonexistent_path")
	{
		CHECK_UNARY(!Directory::exists("/this/path/does/not/exist/at/all"));
	}

	SUBCASE("exists_true_for_real_path")
	{
		CHECK_UNARY(Directory::exists("/tmp"));
	}

	SUBCASE("root_returns_slash")
	{
		CHECK_EQ(Directory::root(), "/");
	}

	SUBCASE("pathDirectory_strips_filename")
	{
		// /tmp exists as a real directory, so the whole path is the directory.
		// For a hypothetical file path we can use cleanPath to strip the file.
		std::string dir = Directory::pathDirectory("/foo/bar/file.xml");
		CHECK_EQ(dir, "/foo/bar");
	}

	SUBCASE("pathDirectory_of_root_is_root")
	{
		CHECK_EQ(Directory::pathDirectory("/"), "/");
	}

	SUBCASE("cwd_is_nonempty")
	{
		std::string cwd = Directory::cwd();
		REQUIRE(!cwd.empty());
		CHECK_EQ('/', cwd[0]); // must be an absolute path
	}

	SUBCASE("seperator_returns_slash")
	{
		// Exercise the seperator() function so its line is hit.
		Directory d("/tmp");
		CHECK_EQ(std::string("/"), d.seperator());
	}

	SUBCASE("cleanPath_dotdot_at_root_does_not_go_above_root")
	{
		// "/../foo" should normalise to "/foo" without popping the empty
		// path.  This exercises the false branch of
		// `if(!path.empty())` in parsePath.
		CHECK_EQ(Directory::cleanPath("/../foo"), "/foo");
	}

	SUBCASE("pathDirectory_of_directory_returns_itself")
	{
		// When the filepath IS a directory, pathDirectory must return it
		// unchanged.  This covers the isDir(filepath) == true branch.
		std::string result = Directory::pathDirectory("/tmp");
		CHECK_EQ(result, "/tmp");
	}

	SUBCASE("pathDirectory_of_empty_string_returns_root")
	{
		// An empty filepath is not a directory and parsePath("") yields an
		// empty path vector, exercising the `path.size() == 0` false branch
		// of pathDirectory's second conditional.
		std::string result = Directory::pathDirectory("");
		CHECK_EQ(result, "/");
	}
}

#ifndef _WIN32
TEST_CASE_FIXTURE(TempDirFixture, "DirectoryInstanceTest")
{
	SUBCASE("constructor_sets_path")
	{
		// Constructing a Directory immediately makes path() return the cleaned
		// path of the directory passed in.
		Directory d(base_path);
		CHECK_EQ(base_path, d.path());
	}

	SUBCASE("isDir_instance_method_true_for_directory")
	{
		Directory d(base_path);
		CHECK_UNARY(d.isDir());
	}

	SUBCASE("isDir_instance_method_false_for_file_path")
	{
		// Point Directory at one of the XML files – isDir() should return
		// false.
		Directory d(base_path + "/drum.xml");
		CHECK_UNARY(!d.isDir());
	}

	SUBCASE("count_and_entryList_reflect_directory_contents")
	{
		// After construction, the entry list should contain the two
		// visible subdirectories ("..", "/alpha", "/beta") and the two
		// XML files ("drum.xml", "kit.xml").  Hidden entries and non-XML
		// files are filtered by listFiles.
		Directory d(base_path);
		auto entries = d.entryList();

		// Verify that count() agrees with the actual list size.
		CHECK_EQ(d.count(), entries.size());

		// We must find "..", "/alpha", "/beta", "drum.xml", "kit.xml".
		auto has = [&](const std::string& name)
		{
			return std::any_of(entries.begin(), entries.end(),
			    [&](const std::string& entry) { return entry == name; });
		};

		CHECK_UNARY(has(".."));
		CHECK_UNARY(has("/alpha"));
		CHECK_UNARY(has("/beta"));
		CHECK_UNARY(has("drum.xml"));
		CHECK_UNARY(has("kit.xml"));

		// Hidden directory and non-XML files must NOT appear.
		CHECK_UNARY(!has("/.hidden"));
		CHECK_UNARY(!has(".hidden"));
		CHECK_UNARY(!has("readme.txt"));
		CHECK_UNARY(!has("x"));
	}

	SUBCASE("setPath_changes_directory")
	{
		// Start in the base directory, then change to alpha/ via setPath.
		Directory d(base_path);
		std::string alpha_path = base_path + "/alpha";
		d.setPath(alpha_path);
		CHECK_EQ(alpha_path, d.path());
	}

	SUBCASE("refresh_repopulates_entry_list")
	{
		Directory d(base_path);
		std::size_t before = d.count();
		// Create a new XML file and refresh to pick it up.
		std::string new_file = base_path + "/new.xml";
		create_empty_file(new_file);
		d.refresh();
		CHECK_EQ(before + 1, d.count());
		// Clean up
		unlink(new_file.c_str());
	}

	SUBCASE("cd_empty_string_returns_true_no_change")
	{
		Directory d(base_path);
		CHECK_UNARY(d.cd(""));
		CHECK_EQ(base_path, d.path());
	}

	SUBCASE("cd_dot_returns_true_no_change")
	{
		Directory d(base_path);
		CHECK_UNARY(d.cd("."));
		CHECK_EQ(base_path, d.path());
	}

	SUBCASE("cd_valid_subdir_returns_true_and_changes_path")
	{
		Directory d(base_path);
		CHECK_UNARY(d.cd("alpha"));
		CHECK_EQ(base_path + "/alpha", d.path());
	}

	SUBCASE("cd_nonexistent_subdir_returns_false")
	{
		Directory d(base_path);
		CHECK_UNARY(!d.cd("nonexistent_subdir_xyz"));
		// Path must stay unchanged on failure.
		CHECK_EQ(base_path, d.path());
	}

	SUBCASE("cdUp_moves_to_parent")
	{
		Directory d(base_path + "/alpha");
		CHECK_UNARY(d.cdUp());
		CHECK_EQ(base_path, d.path());
	}

	SUBCASE("fileExists_true_for_existing_file")
	{
		Directory d(base_path);
		// fileExists checks path + SEP + filename; drum.xml is not a directory.
		CHECK_UNARY(d.fileExists("drum.xml"));
	}

	SUBCASE("fileExists_false_for_directory")
	{
		// fileExists returns !isDir(path+name), so a subdirectory yields false.
		Directory d(base_path);
		CHECK_UNARY(!d.fileExists("alpha"));
	}

	SUBCASE("listFiles_without_hidden_filter_includes_hidden_dirs")
	{
		// Calling listFiles with filter=0 should include the hidden directory
		// (it is a directory, so its hidden flag does not get ANDed with
		// filter=0).
		auto entries = Directory::listFiles(base_path, 0);

		auto has = [&](const std::string& name)
		{
			return std::any_of(entries.begin(), entries.end(),
			    [&](const std::string& entry) { return entry == name; });
		};

		// With filter=0, !(entryinfo && 0) is always true, so hidden dirs
		// appear.
		CHECK_UNARY(has("/.hidden"));
	}

	SUBCASE("listFiles_on_nonexistent_path_returns_empty")
	{
		auto entries =
		    Directory::listFiles("/this/path/does/not/exist/at/all", 0);
		CHECK_UNARY(entries.empty());
	}
}
#endif // _WIN32
