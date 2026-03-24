/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configtest.cc
 *
 *  Thu May 14 20:58:29 CEST 2015
 *  Copyright 2015 Bent Bisballe Nyeng
 *  deva@aasimon.org
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

#include <stdio.h>
#include <unistd.h>

#include "../src/configfile.h"

class TestConfigFile : public ConfigFile
{
public:
	TestConfigFile() : ConfigFile("")
	{
	}

protected:
	// Overload the built-in open method to use local file instead of homedir.
	bool open(std::ios_base::openmode mode) override
	{
		current_file.open("test.conf", mode);
		return current_file.is_open();
	}
};

struct test_configtestFixture
{
	~test_configtestFixture()
	{
		unlink("test.conf");
	}

	void writeFile(const std::string& content, bool newline = true)
	{
		std::fstream file("test.conf", std::ios_base::out);
		file << content;
		if(newline)
		{
			file << std::endl;
		}
		file.close();
	}
};

TEST_CASE_FIXTURE(test_configtestFixture, "test_configtest")
{
	SUBCASE("loading_no_newline")
	{
		writeFile("a:b", false);

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_equal_sign")
	{
		writeFile(" a =\tb\t");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_newline")
	{
		writeFile("a:b");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_padding_space")
	{
		writeFile(" a : b ", false);

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_padding_tab")
	{
		writeFile("\ta\t:\tb\t", false);

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_padding_space_newline")
	{
		writeFile(" a : b ");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_padding_tab_newline")
	{
		writeFile("\ta\t:\tb\t");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_comment")
	{
		writeFile("# comment\na:b");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_inline_comment")
	{
		writeFile("a:b #comment");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("b"), cf.getValue("a"));
	}

	SUBCASE("loading_single_quoted_string")
	{
		writeFile("a: '#\"b\" ' ");

		TestConfigFile cf;
		CHECK(cf.load());
		CHECK_EQ(std::string("#\"b\" "), cf.getValue("a"));
	}

	SUBCASE("loading_double_quoted_string")
	{
		writeFile("a: \"#'b' \" ");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string("#'b' "), cf.getValue("a"));
	}

	SUBCASE("loading_error_no_key")
	{
		writeFile(":foo");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
	}

	SUBCASE("loading_error_no_value")
	{
		writeFile("key");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
	}

	SUBCASE("loading_error_string_not_terminated_single")
	{
		writeFile("a:'b");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
	}

	SUBCASE("loading_error_string_not_terminated_double")
	{
		writeFile("a:\"b");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
	}

	SUBCASE("empty_value")
	{
		writeFile("a:");

		TestConfigFile cf;
		CHECK_EQ(true, cf.load());
		CHECK_EQ(std::string(""), cf.getValue("a"));
	}

	SUBCASE("loading_error_bad_symbol_after_key")
	{
		writeFile("alpha beta:42");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
		CHECK_EQ(std::string(""), cf.getValue("alpha"));
	}

	SUBCASE("loading_error_bad_symbol_after_value")
	{
		writeFile("alpha:42 trailing");

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
		CHECK_EQ(std::string(""), cf.getValue("alpha"));
	}

	SUBCASE("loading_error_stops_at_first_bad_line")
	{
		writeFile("good:one\nbroken line\nignored:two", false);

		TestConfigFile cf;
		CHECK_EQ(false, cf.load());
		CHECK_EQ(std::string("one"), cf.getValue("good"));
		CHECK_EQ(std::string(""), cf.getValue("ignored"));
	}

	SUBCASE("loading_error_clears_stale_values")
	{
		writeFile("broken line", false);

		TestConfigFile cf;
		cf.setValue("stale", "value");
		CHECK_EQ(false, cf.load());
		CHECK_EQ(std::string(""), cf.getValue("stale"));
	}
}
