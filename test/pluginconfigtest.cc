/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pluginconfigtest.cc
 *
 *  Tue Mar 24 18:00:00 CET 2026
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

#include <cstdio>

#include <plugingui/pluginconfig.h>

// A subclass that redirects file I/O to a controlled temporary file so that
// tests do not touch the user's home directory.
class TestableConfig : public GUI::Config
{
public:
	// After the base constructor runs (and its auto-load silently fails because
	// the temp file does not exist yet), we are fully constructed with a valid
	// vtable so subsequent explicit load()/save() calls will use our override.
	explicit TestableConfig(const std::string& path) : test_path(path)
	{
	}

	// Reload using the temp file
	bool loadFromFile()
	{
		return GUI::Config::load();
	}

	// Save using the temp file
	bool saveToFile()
	{
		return GUI::Config::save();
	}

protected:
	bool open(std::ios_base::openmode mode) override
	{
		current_file.open(test_path, mode);
		return current_file.is_open();
	}

private:
	std::string test_path;
};

struct PluginConfigFixture
{
	static constexpr const char* tmp = "/tmp/pluginconfigtest.conf";

	~PluginConfigFixture()
	{
		std::remove(tmp);
	}
};

TEST_CASE_FIXTURE(PluginConfigFixture, "PluginConfigTest")
{
	SUBCASE("default_kit_path_empty_when_file_missing")
	{
		// File does not exist yet – load should return false and
		// defaultKitPath should be left empty.
		TestableConfig cfg(tmp);
		// Constructor silently tries real home dir (won't find anything) so
		// defaultKitPath is empty; now reload from our temp path (also empty).
		CHECK_UNARY(!cfg.loadFromFile());
		CHECK_EQ(std::string(""), cfg.defaultKitPath);
	}

	SUBCASE("save_and_reload_roundtrip")
	{
		TestableConfig cfg(tmp);
		cfg.defaultKitPath = "/usr/share/drumkits/example.xml";
		CHECK(cfg.saveToFile());

		// Reload into a fresh instance pointing at the same temp file.
		TestableConfig cfg2(tmp);
		CHECK_UNARY(cfg2.loadFromFile());
		CHECK_EQ(std::string("/usr/share/drumkits/example.xml"),
		    cfg2.defaultKitPath);
	}

	SUBCASE("save_overwrites_previous_value")
	{
		TestableConfig cfg(tmp);
		cfg.defaultKitPath = "/first/path.xml";
		cfg.saveToFile();

		cfg.defaultKitPath = "/second/path.xml";
		cfg.saveToFile();

		TestableConfig cfg2(tmp);
		CHECK_UNARY(cfg2.loadFromFile());
		CHECK_EQ(std::string("/second/path.xml"), cfg2.defaultKitPath);
	}

	SUBCASE("load_clears_previous_default_kit_path")
	{
		// Write a file with a known path
		TestableConfig writer(tmp);
		writer.defaultKitPath = "/some/kit.xml";
		writer.saveToFile();

		// Load it into a reader
		TestableConfig reader(tmp);
		CHECK_UNARY(reader.loadFromFile());
		CHECK_EQ(std::string("/some/kit.xml"), reader.defaultKitPath);

		// Now remove the file and reload – defaultKitPath should be cleared
		std::remove(tmp);
		reader.defaultKitPath = "stale";
		CHECK_UNARY(!reader.loadFromFile()); // missing file: must return false
		CHECK_EQ(std::string(""), reader.defaultKitPath);
	}
}
