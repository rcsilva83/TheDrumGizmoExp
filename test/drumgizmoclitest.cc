/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumgizmoclitest.cc
 *
 *  Tue Mar 24 16:10:00 CET 2026
 *  Copyright 2026 DrumGizmo team
 *
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

#include "drumkit_creator.h"
#include "scopedfile.h"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/wait.h>
#endif

struct CommandResult
{
	int exit_code;
	std::string output;
};

static std::string shellEscape(const std::string& arg)
{
#ifndef _WIN32
	std::string escaped = "'";
	for(const auto ch : arg)
	{
		if(ch == '\'')
		{
			escaped += "'\\''";
		}
		else
		{
			escaped += ch;
		}
	}
	escaped += "'";
	return escaped;
#else
	std::string escaped = "\"";
	for(const auto ch : arg)
	{
		if(ch == '"')
		{
			escaped += "\\\"";
		}
		else
		{
			escaped += ch;
		}
	}
	escaped += "\"";
	return escaped;
#endif
}

static CommandResult runDrumgizmoCli(const std::vector<std::string>& args)
{
	std::string command = shellEscape(DRUMGIZMO_CLI_BIN);
	for(const auto& arg : args)
	{
		command += " ";
		command += shellEscape(arg);
	}

	ScopedFile output_file("");
	command += " >";
	command += shellEscape(output_file.filename());
	command += " 2>&1";

	auto status = std::system(command.c_str());
	int exit_code = status;
#ifndef _WIN32
	if(WIFEXITED(status))
	{
		exit_code = WEXITSTATUS(status);
	}
#endif

	std::ifstream stream(output_file.filename());
	std::string output((std::istreambuf_iterator<char>(stream)),
	    std::istreambuf_iterator<char>());

	return {exit_code, output};
}

struct DrumgizmoCliFixture
{
	DrumgizmoCliFixture()
	{
		kitfile = drumkit_creator.createStdKit("cli_kit");
	}

	DrumkitCreator drumkit_creator;
	std::string kitfile;
};

TEST_CASE_FIXTURE(DrumgizmoCliFixture, "DrumgizmoCli")
{
	SUBCASE("noArgsPrintsVersionAndUsage")
	{
		auto result = runDrumgizmoCli({});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("DrumGizmo v"));
		CHECK_NE(std::string::npos, result.output.find("Usage:"));
	}

	SUBCASE("helpPrintsUsage")
	{
		auto result = runDrumgizmoCli({"--help"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Usage:"));
		CHECK_NE(std::string::npos, result.output.find("Options:"));
	}

	SUBCASE("missingKitfileReturnsError")
	{
		auto result = runDrumgizmoCli(
		    {"--inputengine", "dummy", "--outputengine", "dummy"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Missing kitfile."));
	}

	SUBCASE("missingInputEngineReturnsError")
	{
		auto result = runDrumgizmoCli({"--outputengine", "dummy", kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(
		    std::string::npos, result.output.find("No input engine selected."));
	}

	SUBCASE("missingOutputEngineReturnsError")
	{
		auto result = runDrumgizmoCli({"--inputengine", "dummy", kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("No output engine selected."));
	}

	SUBCASE("invalidInputEngineReturnsError")
	{
		auto result = runDrumgizmoCli({"--inputengine", "not-an-engine",
		    "--outputengine", "dummy", kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Invalid input engine: not-an-engine"));
	}

	SUBCASE("missingKitfileOnDiskReturnsError")
	{
		auto result = runDrumgizmoCli({"--inputengine", "dummy",
		    "--outputengine", "dummy", "/tmp/dg-does-not-exist.xml"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Can not open /tmp/dg-does-not-exist.xml"));
	}

	SUBCASE("multipleKitfilesReturnsError")
	{
		auto result = runDrumgizmoCli({"--inputengine", "dummy",
		    "--outputengine", "dummy", kitfile, kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Can only handle a single kitfile."));
	}

	SUBCASE("dummyInputAndOutputRunSucceeds")
	{
		auto result = runDrumgizmoCli({"--inputengine", "dummy",
		    "--outputengine", "dummy", "--endpos", "1", kitfile});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Using kitfile:"));
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}
}
