/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            dgvalidatortest.cc
 *
 *  Tue Apr 21 2026
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

#include <config.h>

#include "clitestutils.h"
#include "drumkit_creator.h"

#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

static CommandResult runDgvalidator(const std::vector<std::string>& args)
{
	return runCommand(DGVALIDATOR_BIN, args);
}

struct DgvalidatorFixture
{
	DgvalidatorFixture()
	{
		kitfile = drumkit_creator.createStdKit("validator_kit");
	}

	DrumkitCreator drumkit_creator;
	std::string kitfile;
};

TEST_CASE_FIXTURE(DgvalidatorFixture, "DgvalidatorCli")
{
	SUBCASE("helpPrintsUsage")
	{
		auto result = runDgvalidator({"--help"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Usage:"));
		CHECK_NE(std::string::npos, result.output.find("Options:"));
	}

	SUBCASE("versionPrintsVersion")
	{
		auto result = runDgvalidator({"--version"});
		CHECK_EQ(0, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("DGValidator v"));
	}

	SUBCASE("missingKitfileReturnsError")
	{
		auto result = runDgvalidator({});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos, result.output.find("Missing kitfile."));
	}

	SUBCASE("missingKitfileOnDiskReturnsError")
	{
		// Generate a unique non-existent path using process ID and timestamp
		auto time_val = static_cast<long>(time(nullptr));
		auto pid_val = static_cast<long>(getpid());
		std::string nonexistent_path = "/tmp/dgvalidator-test-nonexistent-" +
		                               std::to_string(pid_val) + "-" +
		                               std::to_string(time_val) + ".xml";
		// Ensure file does not exist (in case of collision, though extremely
		// unlikely)
		(void)unlink(nonexistent_path.c_str());
		auto result = runDgvalidator({nonexistent_path});
		CHECK_EQ(1, result.exit_code);
		std::string expected_error =
		    "XML parse error in '" + nonexistent_path + "'";
		CHECK_NE(std::string::npos, result.output.find(expected_error));
	}

	SUBCASE("validKitWithNoAudioReturnsSuccess")
	{
		auto result = runDgvalidator({"--no-audio", kitfile});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("pedanticTreatsMissingMetadataAsError")
	{
		auto result = runDgvalidator({"--no-audio", "--pedantic", kitfile});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(
		    std::string::npos, result.output.find("Missing version field."));
	}

	SUBCASE("quietWithMissingKitfileReturnsError")
	{
		auto result = runDgvalidator({"--quiet"});
		CHECK_EQ(1, result.exit_code);
		// The direct cerr "Missing kitfile." should still appear
		CHECK_NE(std::string::npos, result.output.find("Missing kitfile."));
	}

	SUBCASE("quietOptionWithNoAudioValidKit")
	{
		auto result = runDgvalidator({"--quiet", "--no-audio", kitfile});
		// With --quiet, no logger output, but no-audio skips audio checks.
		// Metadata warnings are suppressed by quiet.
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("verboseOptionShowsWarningsAndInfo")
	{
		// -v increments verbosity to 2, -vv to 3
		auto result = runDgvalidator({"-v", "--no-audio", kitfile});
		CHECK_EQ(0, result.exit_code);
		// With -v (verbosity=2), warnings become visible
		CHECK_NE(std::string::npos, result.output.find("[Warning]"));
	}

	SUBCASE("verboseTwiceShowsInfoMessages")
	{
		auto result = runDgvalidator({"-v", "-v", "--no-audio", kitfile});
		CHECK_EQ(0, result.exit_code);
		// With -vv (verbosity=3), info messages become visible
		CHECK_NE(std::string::npos, result.output.find("[Info]"));
	}

	SUBCASE("verboseAndPedanticTreatsWarningsAsErrors")
	{
		// -v shows warnings, -p makes warnings errors
		auto result =
		    runDgvalidator({"-v", "--pedantic", "--no-audio", kitfile});
		CHECK_EQ(1, result.exit_code);
		// Missing metadata (e.g. version) is logged as warning in non-pedantic,
		// but as error in pedantic mode.
		CHECK_NE(
		    std::string::npos, result.output.find("Validator found errors."));
	}

	SUBCASE("validKitWithAudioReturnsSuccess")
	{
		auto result = runDgvalidator({kitfile});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithMissingAudioFilesReturnsError")
	{
		auto broken_kit = drumkit_creator.createKitWithBrokenAudio("broken_audio");
		auto result = runDgvalidator({broken_kit});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(std::string::npos,
		    result.output.find("Error loading audio file"));
	}

	SUBCASE("kitWithBrokenInstrumentReturnsError")
	{
		auto broken_kit =
		    drumkit_creator.createKitWithBrokenInstrument("broken_instr");
		auto result = runDgvalidator({"--no-audio", broken_kit});
		CHECK_EQ(1, result.exit_code);
	}

	SUBCASE("kitWithAllMetadataReturnsSuccess")
	{
		auto metadata_kit = drumkit_creator.createKitWithMetadata("meta_kit",
		    "1.0", "Test Kit", "CC-BY", "Test Author", "test@example.com");
		auto result = runDgvalidator({"--no-audio", metadata_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithImageReturnsSuccess")
	{
		auto image_kit = drumkit_creator.createKitWithMetadata("image_kit",
		    "1.0", "Test Kit", "CC-BY", "Test Author", "test@example.com",
		    true, false, false);
		auto result = runDgvalidator({"-v", "-v", "--no-audio", image_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithImageMapReturnsSuccess")
	{
		auto image_map_kit = drumkit_creator.createKitWithMetadata(
		    "image_map_kit", "1.0", "Test Kit", "CC-BY", "Test Author",
		    "test@example.com", false, true, false);
		auto result =
		    runDgvalidator({"-v", "-v", "--no-audio", image_map_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithImageAndImageMapReturnsSuccess")
	{
		auto both_kit = drumkit_creator.createKitWithMetadata("both_kit", "1.0",
		    "Test Kit", "CC-BY", "Test Author", "test@example.com", true, true,
		    false);
		auto result = runDgvalidator({"-v", "-v", "--no-audio", both_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithLogoReturnsSuccess")
	{
		auto logo_kit = drumkit_creator.createKitWithMetadata("logo_kit", "1.0",
		    "Test Kit", "CC-BY", "Test Author", "test@example.com", false,
		    false, true);
		auto result = runDgvalidator({"-v", "-v", "--no-audio", logo_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithMissingMetadataFieldsShowsWarnings")
	{
		auto partial_kit = drumkit_creator.createKitWithMetadata("partial_kit",
		    "", "", "", "", "");
		auto result = runDgvalidator({"-v", "--no-audio", partial_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("kitWithPartialMetadataShowsSomeWarnings")
	{
		auto partial_kit = drumkit_creator.createKitWithMetadata("partial_kit2",
		    "1.0", "", "CC-BY", "", "test@example.com");
		auto result = runDgvalidator({"-v", "--no-audio", partial_kit});
		CHECK_EQ(0, result.exit_code);
	}

	SUBCASE("pedanticWithMissingMetadataReturnsError")
	{
		auto partial_kit = drumkit_creator.createKitWithMetadata("partial_kit3",
		    "", "", "", "", "");
		auto result =
		    runDgvalidator({"--pedantic", "--no-audio", partial_kit});
		CHECK_EQ(1, result.exit_code);
		CHECK_NE(
		    std::string::npos, result.output.find("Validator found errors."));
	}
}
