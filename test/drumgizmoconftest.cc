/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumgizmoconftest.cc
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "../src/drumgizmoconf.h"

struct DrumgizmoConfigFixture
{
	std::string original_home;
	std::string tmpdir;

	DrumgizmoConfigFixture()
	{
		auto* home_env = getenv("HOME");
		original_home = home_env ? home_env : "";

		tmpdir = "/tmp/dg_conf_test_" + std::to_string(getpid());
		mkdir(tmpdir.c_str(), 0755);
		setenv("HOME", tmpdir.c_str(), 1);
	}

	~DrumgizmoConfigFixture()
	{
		// Restore original HOME
		if(original_home.empty())
		{
			unsetenv("HOME");
		}
		else
		{
			setenv("HOME", original_home.c_str(), 1);
		}

		// Cleanup temp directory
		std::string config_dir = tmpdir + "/.drumgizmo";
		std::string config_file = config_dir + "/drumgizmo.conf";
		std::remove(config_file.c_str());
		rmdir(config_dir.c_str());
		rmdir(tmpdir.c_str());
	}

	void writeConfig(const std::string& kit, const std::string& midimap)
	{
		std::string config_dir = tmpdir + "/.drumgizmo";
		mkdir(config_dir.c_str(), 0755);
		std::string config_file = config_dir + "/drumgizmo.conf";
		std::ofstream out(config_file);
		out << "defaultKit = \"" << kit << "\"\n";
		out << "defaultMidimap = \"" << midimap << "\"\n";
		out.close();
	}
};

TEST_CASE_FIXTURE(DrumgizmoConfigFixture, "drumgizmoconf_empty_defaults")
{
	SUBCASE("no_config_file_means_empty_defaults")
	{
		DrumgizmoConfig conf;
		CHECK_EQ(std::string(""), conf.defaultKit);
		CHECK_EQ(std::string(""), conf.defaultMidimap);
	}
}

TEST_CASE_FIXTURE(DrumgizmoConfigFixture, "drumgizmoconf_loaded_values")
{
	SUBCASE("config_file_with_kit_and_midimap")
	{
		writeConfig("/tmp/my/kit.xml", "/tmp/my/map.xml");

		DrumgizmoConfig conf;
		CHECK_EQ(std::string("/tmp/my/kit.xml"), conf.defaultKit);
		CHECK_EQ(std::string("/tmp/my/map.xml"), conf.defaultMidimap);
	}

	SUBCASE("config_file_with_kit_only")
	{
		writeConfig("/only/kit.xml", "");

		DrumgizmoConfig conf;
		CHECK_EQ(std::string("/only/kit.xml"), conf.defaultKit);
		CHECK_EQ(std::string(""), conf.defaultMidimap);
	}

	SUBCASE("config_file_with_midimap_only")
	{
		writeConfig("", "/only/map.xml");

		DrumgizmoConfig conf;
		CHECK_EQ(std::string(""), conf.defaultKit);
		CHECK_EQ(std::string("/only/map.xml"), conf.defaultMidimap);
	}
}

TEST_CASE_FIXTURE(DrumgizmoConfigFixture, "drumgizmoconf_save_and_reload")
{
	SUBCASE("set_defaults_and_reload")
	{
		// First create a base config so load() succeeds and doesn't clear
		writeConfig("original_kit.xml", "original_map.xml");

		// Load it, modify, save, and verify a reload sees new values
		{
			DrumgizmoConfig conf;
			CHECK_EQ(std::string("original_kit.xml"), conf.defaultKit);
			// Note: modifying and saving -- destructor will also save
			conf.defaultKit = "new_kit.xml";
			conf.defaultMidimap = "new_map.xml";
			conf.save();
		}

		// Reload the config file
		DrumgizmoConfig conf2;
		CHECK_EQ(std::string("new_kit.xml"), conf2.defaultKit);
		CHECK_EQ(std::string("new_map.xml"), conf2.defaultMidimap);
	}
}
