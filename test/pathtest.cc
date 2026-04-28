/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pathtest.cc
 *
 *  Mon Apr 28 12:00:00 UTC 2026
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

#include "../src/path.h"

TEST_CASE("pathtest_getPath")
{
	SUBCASE("absolute_path_with_file")
	{
		auto result = getPath("/usr/local/bin/program");
		CHECK_EQ("/usr/local/bin", result);
	}

	SUBCASE("relative_path_with_file")
	{
		auto result = getPath("foo/bar/baz.txt");
		CHECK_EQ("foo/bar", result);
	}

	SUBCASE("filename_only")
	{
		auto result = getPath("file.txt");
		CHECK_EQ(".", result);
	}

	SUBCASE("directory_with_trailing_slash")
	{
		auto result = getPath("/tmp/dir/");
		// dirname strips trailing slashes and returns parent
		CHECK_EQ("/tmp", result);
	}

	SUBCASE("empty_string")
	{
		auto result = getPath("");
		CHECK_EQ(".", result);
	}
}

TEST_CASE("pathtest_getFile")
{
	SUBCASE("absolute_path_with_file")
	{
		auto result = getFile("/usr/local/bin/program");
		CHECK_EQ("program", result);
	}

	SUBCASE("relative_path_with_file")
	{
		auto result = getFile("foo/bar/baz.txt");
		CHECK_EQ("baz.txt", result);
	}

	SUBCASE("filename_only")
	{
		auto result = getFile("file.txt");
		CHECK_EQ("file.txt", result);
	}

	SUBCASE("hidden_file")
	{
		auto result = getFile("/home/user/.config");
		CHECK_EQ(".config", result);
	}

	SUBCASE("empty_string")
	{
		auto result = getFile("");
		// basename("") returns "." on POSIX
		CHECK_EQ(".", result);
	}
}
