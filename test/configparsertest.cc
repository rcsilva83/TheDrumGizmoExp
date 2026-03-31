/* -*- Mode: c++ -*- */
/***************************************************************************
 *            configparsertest.cc
 *
 *  Wed Jul 25 19:08:40 CEST 2018
 *  Copyright 2018 Bent Bisballe Nyeng
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

#include <configparser.h>

#include <vector>

TEST_CASE("ConfigParserTest")
{
	SUBCASE("test")
	{
		std::string xml = "<?xml version='1.0' encoding='UTF-8'?>\n"
		                  "<config>\n"
		                  "  <value name=\"foo\">42</value>\n"
		                  "  <value name=\"bar\">true</value>\n"
		                  "  <value name=\"bas\">&quot;&lt;</value>\n"
		                  "</config>";

		ConfigParser parser;
		CHECK(parser.parseString(xml));

		CHECK_EQ(std::string("42"), parser.value("foo", "-"));
		CHECK_EQ(std::string("true"), parser.value("bar", "-"));
		CHECK_EQ(std::string("\"<"), parser.value("bas", "-"));

		// Non-existing value
		CHECK_EQ(std::string("-"), parser.value("bas2", "-"));
	}

	SUBCASE("invalid")
	{
		std::string xml = "<?xml version='1.0' encoding='UTF-8'?>\n"
		                  "<config\n"
		                  "  <value name=\"foo\">42</value>\n"
		                  "  <value name=\"bar\">true</value>\n"
		                  "  <value name=\"bas\">&quot;&lt;</value>\n"
		                  "</config>";

		ConfigParser parser;
		// Epxect parser error (missing '>' in line 2)
		CHECK(!parser.parseString(xml));
	}

	SUBCASE("edgeCaseMatrix")
	{
		struct TestCase
		{
			std::string name;
			std::string xml;
			bool preseed;
			bool expected_status;
			std::string expected_foo;
			std::string expected_missing;
		};

		const std::vector<TestCase> cases = {
		    {"near-valid minimal config",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<config>\n"
		        "  <value name=\"foo\">41</value>\n"
		        "</config>",
		        false, true, "41", "fallback"},
		    {"conflicting duplicates and partial entries",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<config>\n"
		        "  <value>42</value>\n"
		        "  <value name=\"foo\">43</value>\n"
		        "  <value name=\"foo\">44</value>\n"
		        "</config>",
		        false, true, "44", "fallback"},
		    {"malformed xml keeps prior values",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<config\n"
		        "  <value name=\"foo\">broken</value>\n"
		        "</config>",
		        true, false, "seed", "fallback"},
		    {"unsupported version keeps prior values",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<config version=\"2.0\">\n"
		        "  <value name=\"foo\">42</value>\n"
		        "</config>",
		        true, false, "seed", "fallback"},
		    {"wrong root is ignored",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<cfg>\n"
		        "  <value name=\"foo\">42</value>\n"
		        "</cfg>",
		        false, true, "-", "fallback"},
		    {"partial config without target key",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<config>\n"
		        "  <value name=\"bar\">17</value>\n"
		        "</config>",
		        false, true, "-", "fallback"}};

		for(const auto& test_case : cases)
		{
			ConfigParser parser;
			INFO(test_case.name);

			if(test_case.preseed)
			{
				CHECK(parser.parseString(
				    "<?xml version='1.0' encoding='UTF-8'?>\n"
				    "<config>\n"
				    "  <value name=\"foo\">seed</value>\n"
				    "</config>"));
			}

			CHECK_EQ(
			    test_case.expected_status, parser.parseString(test_case.xml));
			CHECK_EQ(test_case.expected_foo, parser.value("foo", "-"));
			CHECK_EQ(test_case.expected_missing,
			    parser.value("missing", "fallback"));
		}
	}

	SUBCASE("recoveryAfterParseFailure")
	{
		ConfigParser parser;

		CHECK(parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                         "<config>\n"
		                         "  <value name=\"foo\">42</value>\n"
		                         "</config>"));
		CHECK_EQ(std::string("42"), parser.value("foo", "-"));

		CHECK(!parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                          "<config\n"
		                          "  <value name=\"foo\">43</value>\n"
		                          "</config>"));

		CHECK_EQ(std::string("42"), parser.value("foo", "-"));
	}

	SUBCASE("unsupportedVersionRejection")
	{
		// A fresh parser with an unsupported version must return false
		// and leave the value map empty.
		ConfigParser parser;
		CHECK(!parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                          "<config version=\"2.0\">\n"
		                          "  <value name=\"foo\">42</value>\n"
		                          "</config>"));
		CHECK_EQ(std::string("-"), parser.value("foo", "-"));
		CHECK_EQ(std::string("default"), parser.value("missing", "default"));
	}

	SUBCASE("missingConfigNode")
	{
		// When the document contains no <config> root element, parseString
		// must succeed (no error) but must not populate any values.
		ConfigParser parser;

		// Root element exists but is not named "config".
		CHECK(parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                         "<settings>\n"
		                         "  <value name=\"foo\">42</value>\n"
		                         "</settings>"));
		CHECK_EQ(std::string("-"), parser.value("foo", "-"));

		// Empty document with a placeholder root but no <config> child.
		CHECK(parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                         "<root/>"));
		CHECK_EQ(std::string("-"), parser.value("foo", "-"));
	}

	SUBCASE("recoveryAfterVersionFailure")
	{
		// Seeded values must survive a subsequent unsupported-version failure.
		ConfigParser parser;

		CHECK(parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                         "<config>\n"
		                         "  <value name=\"foo\">42</value>\n"
		                         "</config>"));
		CHECK_EQ(std::string("42"), parser.value("foo", "-"));

		CHECK(!parser.parseString("<?xml version='1.0' encoding='UTF-8'?>\n"
		                          "<config version=\"2.0\">\n"
		                          "  <value name=\"foo\">99</value>\n"
		                          "</config>"));

		// Prior value must be preserved after the version-rejected parse.
		CHECK_EQ(std::string("42"), parser.value("foo", "-"));
	}
}
