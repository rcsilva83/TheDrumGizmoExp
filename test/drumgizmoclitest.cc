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

static std::vector<std::string> runArgs(const std::string& kitfile)
{
	return {"--inputengine", "dummy", "--outputengine", "dummy",
	    "--endpos", "1", kitfile};
}

// Prepend extra options before the common run args.
static std::vector<std::string> prependArgs(const std::vector<std::string>& prefix,
    const std::string& kitfile)
{
	auto args = runArgs(kitfile);
	args.insert(args.begin(), prefix.begin(), prefix.end());
	return args;
}

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

	SUBCASE("versionPrintsVersionAndCopyright")
	{
		auto result = runDrumgizmoCli({"--version"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("DrumGizmo v"));
		CHECK_NE(std::string::npos, result.output.find("Copyright"));
	}

	SUBCASE("inputengineHelpListsEngines")
	{
		auto result = runDrumgizmoCli({"--inputengine", "help"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Available Input Engines"));
	}

	SUBCASE("outputengineHelpListsEngines")
	{
		auto result = runDrumgizmoCli({"--outputengine", "help"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Available Output Engines"));
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

	SUBCASE("invalidOutputEngineReturnsError")
	{
		// outputengine must be specified before inputengine here because
		// the invalid-output-engine check in the handler is guarded by a
		// condition that requires inputengine to be unset at that point.
		auto result = runDrumgizmoCli(
		    {"--outputengine", "not-an-engine", "--inputengine", "dummy",
		    kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Invalid output engine: not-an-engine"));
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

	SUBCASE("asyncLoadRunSucceeds")
	{
		auto result = runDrumgizmoCli(prependArgs({"--async-load"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("bleedOptionRunSucceeds")
	{
		auto result =
		    runDrumgizmoCli(prependArgs({"--bleed", "0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("noResamplingRunSucceeds")
	{
		auto result =
		    runDrumgizmoCli(prependArgs({"--no-resampling"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("streamingRunSucceeds")
	{
		auto result = runDrumgizmoCli(prependArgs({"--streaming"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("streamingparmsValidLimitRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--streamingparms", "limit=100M"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("streamingparmsInvalidLimitReturnsError")
	{
		auto result = runDrumgizmoCli({"--streamingparms", "limit=0"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Invalid argument for streamparms limit"));
	}

	SUBCASE("streamingparmsUnknownKeyReturnsError")
	{
		auto result = runDrumgizmoCli({"--streamingparms", "unknown=val"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Unknown streamingparms argument unknown"));
	}

	SUBCASE("timingHumanizerRunSucceeds")
	{
		auto result =
		    runDrumgizmoCli(prependArgs({"--timing-humanizer"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("timingHumanizerparmsLaidbackValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--timing-humanizerparms", "laidback=10"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("timingHumanizerparmsLaidbackTooHighReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--timing-humanizerparms", "laidback=200"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("laidback range is +/- 100"));
	}

	SUBCASE("timingHumanizerparmsLaidbackTooLowReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--timing-humanizerparms", "laidback=-200"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("laidback range is +/- 100"));
	}

	SUBCASE("timingHumanizerparmsTightnessValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--timing-humanizerparms", "tightness=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("timingHumanizerparmsTightnessOutOfRangeReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--timing-humanizerparms", "tightness=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("tightness range is [0, 1]"));
	}

	SUBCASE("timingHumanizerparmsRegainValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--timing-humanizerparms", "regain=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("timingHumanizerparmsRegainOutOfRangeReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--timing-humanizerparms", "regain=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("regain range is [0, 1]"));
	}

	SUBCASE("timingHumanizerparmsUnknownKeyReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--timing-humanizerparms", "unknown=val"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Unknown timing-humanizerparms argument unknown"));
	}

	SUBCASE("timingHumanizerparmsMultipleParamsCommaRunSucceeds")
	{
		auto result = runDrumgizmoCli(prependArgs(
		    {"--timing-humanizerparms", "laidback=10,tightness=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("velocityHumanizerRunSucceeds")
	{
		auto result =
		    runDrumgizmoCli(prependArgs({"--velocity-humanizer"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("velocityHumanizerparmsAttackValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(prependArgs(
		    {"--velocity-humanizerparms", "attack=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("velocityHumanizerparmsAttackOutOfRangeReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--velocity-humanizerparms", "attack=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("attack range is [0, 1]"));
	}

	SUBCASE("velocityHumanizerparmsReleaseValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(prependArgs(
		    {"--velocity-humanizerparms", "release=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("velocityHumanizerparmsReleaseOutOfRangeReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--velocity-humanizerparms", "release=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("release range is [0, 1]"));
	}

	SUBCASE("velocityHumanizerparmsStddevValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--velocity-humanizerparms", "stddev=2"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("velocityHumanizerparmsStddevOutOfRangeReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--velocity-humanizerparms", "stddev=5"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("stddev range is [0, 4.5]"));
	}

	SUBCASE("velocityHumanizerparmsUnknownKeyReturnsError")
	{
		auto result =
		    runDrumgizmoCli({"--velocity-humanizerparms", "unknown=val"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find(
		        "Unknown velocity-humanizerparms argument unknown"));
	}

	SUBCASE("voiceLimitRunSucceeds")
	{
		auto result =
		    runDrumgizmoCli(prependArgs({"--voice-limit"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("voiceLimitparmsMaxValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--voice-limitparms", "max=5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("voiceLimitparmsMaxOutOfRangeReturnsError")
	{
		auto result = runDrumgizmoCli({"--voice-limitparms", "max=0"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("max range is [1, 30]"));
	}

	SUBCASE("voiceLimitparmsRampdownValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--voice-limitparms", "rampdown=1.0"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("voiceLimitparmsRampdownOutOfRangeReturnsError")
	{
		auto result = runDrumgizmoCli({"--voice-limitparms", "rampdown=5"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("rampdown range is [0.01, 2.0]"));
	}

	SUBCASE("voiceLimitparmsUnknownKeyReturnsError")
	{
		auto result = runDrumgizmoCli({"--voice-limitparms", "unknown=val"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Unknown voice limitparms argument unknown"));
	}

	SUBCASE("parametersCloseValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--parameters", "close=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("parametersCloseOutOfRangeReturnsError")
	{
		auto result = runDrumgizmoCli({"--parameters", "close=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("close range is [0, 1]"));
	}

	SUBCASE("parametersDiverseValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--parameters", "diverse=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("parametersDiverseOutOfRangeReturnsError")
	{
		auto result = runDrumgizmoCli({"--parameters", "diverse=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("diverse range is [0, 1]"));
	}

	SUBCASE("parametersRandomValidRunSucceeds")
	{
		auto result = runDrumgizmoCli(
		    prependArgs({"--parameters", "random=0.5"}, kitfile));
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}

	SUBCASE("parametersRandomOutOfRangeReturnsError")
	{
		auto result = runDrumgizmoCli({"--parameters", "random=2"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("random range is [0, 1]"));
	}

	SUBCASE("parametersUnknownKeyReturnsError")
	{
		auto result = runDrumgizmoCli({"--parameters", "unknown=val"});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Unknown parameters argument unknown"));
	}

	SUBCASE("endposInvalidValuePrintsErrorAndContinues")
	{
		// Pass invalid endpos first (prints error, endpos stays -1),
		// then a valid endpos=1 so the run loop terminates.
		auto result = runDrumgizmoCli({"--inputengine", "dummy",
		    "--outputengine", "dummy", "--endpos", "not-a-number",
		    "--endpos", "1", kitfile});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Invalid endpos size not-a-number"));
		CHECK_NE(std::string::npos, result.output.find("Quit."));
	}
}
