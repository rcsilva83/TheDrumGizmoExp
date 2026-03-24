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

#include "../src/directory.h"

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
		CHECK_UNARY(!Directory::cwd().empty());
	}
}
